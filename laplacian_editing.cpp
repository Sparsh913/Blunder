// laplacian_editing.cpp
#include "laplacian_editing.h"
#include <vector>
#include <iostream>
#include <limits>
#include <map>
#include <cmath>
#include <queue>

using namespace std;

// Simple sparse matrix implementation
struct Triplet {
    int row, col;
    double value;
    Triplet(int r, int c, double v) : row(r), col(c), value(v) {}
};

class SparseMatrix {
private:
    map<pair<int, int>, double> data;
    int rows, cols;
public:
    SparseMatrix(int r, int c) : rows(r), cols(c) {}
    
    void setFromTriplets(const vector<Triplet>& triplets) {
        for (const auto& t : triplets) {
            data[{t.row, t.col}] = t.value;
        }
    }
    
    double coeff(int row, int col) const {
        auto it = data.find({row, col});
        return (it != data.end()) ? it->second : 0.0;
    }
    
    SparseMatrix transpose() const {
        SparseMatrix result(cols, rows);
        for (const auto& [idx, val] : data) {
            result.data[{idx.second, idx.first}] = val;
        }
        return result;
    }
    
    vector<double> multiply(const vector<double>& x) const {
        vector<double> result(rows, 0.0);
        for (const auto& [idx, val] : data) {
            result[idx.first] += val * x[idx.second];
        }
        return result;
    }
    
    SparseMatrix operator*(const SparseMatrix& rhs) const {
        SparseMatrix result(rows, rhs.cols);
        vector<Triplet> triplets;
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < rhs.cols; ++j) {
                double sum = 0.0;
                for (int k = 0; k < cols; ++k) {
                    sum += coeff(i, k) * rhs.coeff(k, j);
                }
                if (fabs(sum) > 1e-10) {
                    triplets.emplace_back(i, j, sum);
                }
            }
        }
        
        result.setFromTriplets(triplets);
        return result;
    }
    
    SparseMatrix operator+(const SparseMatrix& rhs) const {
        SparseMatrix result(rows, cols);
        result.data = data;
        
        for (const auto& [idx, val] : rhs.data) {
            result.data[idx] += val;
        }
        
        return result;
    }
    
    // Add scalar multiplication operator
    friend SparseMatrix operator*(double scalar, const SparseMatrix& matrix) {
        SparseMatrix result(matrix.rows, matrix.cols);
        for (const auto& [idx, val] : matrix.data) {
            result.data[idx] = scalar * val;
        }
        return result;
    }
    
    int rows_count() const { return rows; }
    int cols_count() const { return cols; }
};

class DenseMatrix {
private:
    vector<vector<double>> data;
public:
    DenseMatrix(int r, int c) : data(r, vector<double>(c, 0.0)) {}
    
    double& operator()(int i, int j) { return data[i][j]; }
    double operator()(int i, int j) const { return data[i][j]; }
    
    int rows() const { return data.size(); }
    int cols() const { return data.empty() ? 0 : data[0].size(); }
    
    vector<double> row(int i) const { return data[i]; }
    void set_row(int i, const YsVec3& vec) {
        data[i][0] = vec.x();
        data[i][1] = vec.y();
        data[i][2] = vec.z();
    }
};

// Optimized Laplacian deformation function using local region
void applyLaplacianDeformation(Mesh &mesh,
                           const unordered_map<int, YsVec3> &control_points,
                           const unordered_set<int> &anchor_points,
                           double lambda) {
    
    // Step 1: Build a vertex ID to handle cache for faster lookups
    unordered_map<int, Mesh::VertexHandle> vtxIdToHandle;
    for (auto vtHd = mesh.FirstVertex(); vtHd != mesh.NullVertex(); mesh.MoveToNext(vtHd)) {
        int vtxId = mesh.GetSearchKey(vtHd);
        vtxIdToHandle[vtxId] = vtHd;
    }
    
    // Step 2: Determine region of influence (vertices to include in the calculation)
    std::unordered_set<int> activeRegion;
    const int MAX_NEIGHBORHOOD_DEPTH = 3; // Number of edge traversals from control points
    
    // Add control points to active region
    for (const auto& [vtxId, _] : control_points) {
        activeRegion.insert(vtxId);
    }
    
    // Add anchor points to active region
    // for (const auto& vtxId : anchor_points) {
    //     activeRegion.insert(vtxId);
    // }
    
    // Breadth-first expansion to add neighborhood around control points
    std::queue<int> frontier;
    std::unordered_set<int> visited = activeRegion;
    
    for (int vtxId : activeRegion) {
        frontier.push(vtxId);
    }
    
    int depth = 0;
    while (!frontier.empty() && depth < MAX_NEIGHBORHOOD_DEPTH) {
        int levelSize = frontier.size();
        
        for (int i = 0; i < levelSize; ++i) {
            int vtxId = frontier.front();
            frontier.pop();
            
            auto vtHdIter = vtxIdToHandle.find(vtxId);
            if (vtHdIter == vtxIdToHandle.end()) continue;
            
            auto vtHd = vtHdIter->second;
            auto neighbors = mesh.GetConnectedVertex(vtHd);
            
            for (const auto& nbVtHd : neighbors) {
                int nbId = mesh.GetSearchKey(nbVtHd);
                if (visited.count(nbId) == 0) {
                    visited.insert(nbId);
                    activeRegion.insert(nbId);
                    frontier.push(nbId);
                }
            }
        }
        
        ++depth;
    }
    
    // Step 3: Create a mapping from global vertex IDs to local indices in our reduced system
    unordered_map<int, int> vtxIdToIdx;
    vector<int> idxToVtxId;
    vector<Mesh::VertexHandle> activeVertices;
    
    int localIdx = 0;
    for (int vtxId : activeRegion) {
        auto vtHdIter = vtxIdToHandle.find(vtxId);
        if (vtHdIter != vtxIdToHandle.end()) {
            vtxIdToIdx[vtxId] = localIdx;
            idxToVtxId.push_back(vtxId);
            activeVertices.push_back(vtHdIter->second);
            localIdx++;
        }
    }
    
    const int n = localIdx; // Size of our reduced system
    
    if (n == 0) {
        cout << "No active vertices found, skipping deformation" << endl;
        return;
    }
    
    cout << "Using " << n << " active vertices out of " << mesh.GetNumVertices() << " total vertices" << endl;
    
    // Step 4: Build Laplacian Matrix for active region only
    vector<Triplet> triplets;
    DenseMatrix delta(n, 3);
    
    for (int i = 0; i < n; ++i) {
        auto vtHd = activeVertices[i];
        auto neighbors = mesh.GetConnectedVertex(vtHd);
        
        // Count neighbors that are in the active region
        int activeNeighborCount = 0;
        for (const auto &nbVtHd : neighbors) {
            int nbId = mesh.GetSearchKey(nbVtHd);
            if (vtxIdToIdx.count(nbId) > 0) {
                activeNeighborCount++;
            }
        }
        
        int deg = activeNeighborCount;
        
        if (deg == 0) {
            // Handle isolated vertices
            triplets.emplace_back(i, i, 1.0);
            
            // Use zero differential coordinates for isolated vertices
            delta(i, 0) = 0.0;
            delta(i, 1) = 0.0;
            delta(i, 2) = 0.0;
            continue;
        }
        
        triplets.emplace_back(i, i, 1.0);
        
        YsVec3 pos = mesh.GetVertexPosition(vtHd);
        YsVec3 lap = pos;
        
        for (const auto &nbVtHd : neighbors) {
            int nbId = mesh.GetSearchKey(nbVtHd);
            auto it = vtxIdToIdx.find(nbId);
            if (it != vtxIdToIdx.end()) {
                int j = it->second;
                triplets.emplace_back(i, j, -1.0 / deg);
                
                // Subtract normalized neighbor position
                YsVec3 nbPos = mesh.GetVertexPosition(nbVtHd);
                YsVec3 scaledNbPos;
                scaledNbPos.SetX(nbPos.x() / deg);
                scaledNbPos.SetY(nbPos.y() / deg);
                scaledNbPos.SetZ(nbPos.z() / deg);
                lap.SetX(lap.x() - scaledNbPos.x());
                lap.SetY(lap.y() - scaledNbPos.y());
                lap.SetZ(lap.z() - scaledNbPos.z());
            }
        }
        
        delta(i, 0) = lap.x();
        delta(i, 1) = lap.y();
        delta(i, 2) = lap.z();
    }
    
    SparseMatrix L(n, n);
    L.setFromTriplets(triplets);
    
    // Step 5: Set up constraints in the reduced system
    int m = 0; // Count valid control points
    for (const auto &[vtxId, _] : control_points) {
        if (vtxIdToIdx.count(vtxId)) {
            m++;
        }
    }
    
    int a = 0; // Count valid anchor points
    for (const auto &vtxId : anchor_points) {
        if (vtxIdToIdx.count(vtxId)) {
            a++;
        }
    }
    
    int totalConstraints = m + a;
    vector<Triplet> constraintTriplets;
    DenseMatrix C(totalConstraints, 3);
    int ci = 0;
    
    // Add control points constraints
    for (const auto &[vtxId, newPos] : control_points) {
        auto it = vtxIdToIdx.find(vtxId);
        if (it == vtxIdToIdx.end()) continue;
        
        int col = it->second;
        constraintTriplets.emplace_back(ci, col, 1.0);
        C(ci, 0) = newPos.x();
        C(ci, 1) = newPos.y();
        C(ci, 2) = newPos.z();
        ++ci;
    }
    
    // Add anchor points constraints
    for (const auto &vtxId : anchor_points) {
        auto it = vtxIdToIdx.find(vtxId);
        if (it == vtxIdToIdx.end()) continue;
        
        int col = it->second;
        constraintTriplets.emplace_back(ci, col, 1.0);
        
        // Find the vertex handle and use its current position
        auto vtHandleIter = vtxIdToHandle.find(vtxId);
        if (vtHandleIter != vtxIdToHandle.end()) {
            YsVec3 pos = mesh.GetVertexPosition(vtHandleIter->second);
            C(ci, 0) = pos.x();
            C(ci, 1) = pos.y();
            C(ci, 2) = pos.z();
        }
        ++ci;
    }
    
    SparseMatrix W(totalConstraints, n);
    W.setFromTriplets(constraintTriplets);

    // Step 6: Create the system matrix: A = L^T L + lambda * W^T W
    SparseMatrix LTranspose = L.transpose();
    SparseMatrix A = LTranspose * L;
    
    if (totalConstraints > 0) {
        SparseMatrix WTranspose = W.transpose();
        SparseMatrix WtW = WTranspose * W;
        SparseMatrix scaledWtW = lambda * WtW;
        A = A + scaledWtW;
    }

    // Step 7: Prepare the right-hand side vector
    vector<vector<double>> b_data(n, vector<double>(3, 0.0));

    // Compute L^T * delta
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double lt_val = L.coeff(j, i); // L^T element (i,j) = L(j,i)
            if (fabs(lt_val) > 1e-10) {
                b_data[i][0] += lt_val * delta(j, 0);
                b_data[i][1] += lt_val * delta(j, 1);
                b_data[i][2] += lt_val * delta(j, 2);
            }
        }
    }

    // Add lambda * W^T * C
    if (totalConstraints > 0) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < totalConstraints; j++) {
                double wt_val = W.coeff(j, i); // W^T element (i,j) = W(j,i)
                if (fabs(wt_val) > 1e-10) {
                    b_data[i][0] += lambda * wt_val * C(j, 0);
                    b_data[i][1] += lambda * wt_val * C(j, 1);
                    b_data[i][2] += lambda * wt_val * C(j, 2);
                }
            }
        }
    }

    // Step 8: Solve the system using Gauss-Seidel (faster than Jacobi)
    vector<vector<double>> X(n, vector<double>(3, 0.0));

    // Initialize X with current positions
    for (int i = 0; i < n; i++) {
        auto vtHd = activeVertices[i];
        YsVec3 pos = mesh.GetVertexPosition(vtHd);
        X[i][0] = pos.x();
        X[i][1] = pos.y();
        X[i][2] = pos.z();
    }

    // Iterate to solve the system
    const int max_iterations = 50; // Reduced iterations for Gauss-Seidel
    const double tolerance = 1e-5;
    bool converged = false;

    for (int iter = 0; iter < max_iterations && !converged; iter++) {
        double error = 0.0;
        
        // For each component (x, y, z)
        for (int comp = 0; comp < 3; comp++) {
            // For each row
            for (int i = 0; i < n; i++) {
                double oldValue = X[i][comp];
                double sum = b_data[i][comp];
                double diag = A.coeff(i, i);
                
                if (fabs(diag) < 1e-10) {
                    cerr << "Diagonal element too small, solver may fail." << endl;
                    diag = 1e-10;
                }
                
                for (int j = 0; j < n; j++) {
                    if (i != j) {
                        double a_val = A.coeff(i, j);
                        if (fabs(a_val) > 1e-10) {
                            sum -= a_val * X[j][comp]; // Uses latest values (Gauss-Seidel)
                        }
                    }
                }
                
                X[i][comp] = sum / diag;
                error += fabs(X[i][comp] - oldValue);
            }
        }
        
        if (error < tolerance * n) {
            converged = true;
            cout << "Solver converged after " << iter+1 << " iterations." << endl;
        }
    }

    if (!converged) {
        cerr << "Warning: Solver did not fully converge." << endl;
    }

    // Step 9: Update only the active vertices in the mesh
    for (int i = 0; i < n; ++i) {
        YsVec3 newPos;
        newPos.SetX(X[i][0]);
        newPos.SetY(X[i][1]);
        newPos.SetZ(X[i][2]);
        mesh.SetVertexPosition(activeVertices[i], newPos);
    }
    
    cout << "Laplacian deformation applied successfully to local region." << endl;
}

// LaplacianEditor implementation
LaplacianEditor::LaplacianEditor(Mesh* mesh) : mesh(mesh) {
    storeOriginalPositions();
    cacheVertexHandles();
}

void LaplacianEditor::cacheVertexHandles() {
    vtxIdToHandle.clear();
    for (auto vtHd = mesh->FirstVertex(); vtHd != mesh->NullVertex(); mesh->MoveToNext(vtHd)) {
        int vtxId = mesh->GetSearchKey(vtHd);
        vtxIdToHandle[vtxId] = vtHd;
    }
}

Mesh::VertexHandle LaplacianEditor::findVertexById(int vtxId) const {
    auto it = vtxIdToHandle.find(vtxId);
    if (it != vtxIdToHandle.end()) {
        return it->second;
    }
    return mesh->NullVertex();
}

bool LaplacianEditor::selectVertex(const YsVec3& position, double threshold) {
    Mesh::VertexHandle closest_vertex = mesh->NullVertex();
    double min_distance = std::numeric_limits<double>::max();
    
    // Find the vertex closest to the given position
    for (auto vtHd = mesh->FirstVertex(); vtHd != mesh->NullVertex(); mesh->MoveToNext(vtHd)) {
        YsVec3 vertex_pos = mesh->GetVertexPosition(vtHd);
        double dx = vertex_pos.x() - position.x();
        double dy = vertex_pos.y() - position.y();
        double dz = vertex_pos.z() - position.z();
        double distance = sqrt(dx*dx + dy*dy + dz*dz);
        
        if (distance < min_distance) {
            min_distance = distance;
            closest_vertex = vtHd;
        }
    }
    
    // Select the vertex if it's within the threshold
    if (closest_vertex != mesh->NullVertex() && min_distance <= threshold) {
        int vtx_id = mesh->GetSearchKey(closest_vertex);
        selected_vertices.insert(vtx_id);
        control_points[vtx_id] = mesh->GetVertexPosition(closest_vertex);
        cout << "Selected vertex " << vtx_id << endl;
        return true;
    }
    
    return false;
}

void LaplacianEditor::createAnchors(const Mesh::VertexHandle& centerVtHd, double radius) {
    // Clear previous anchors
    anchor_vertices.clear();
    
    if (centerVtHd == mesh->NullVertex()) {
        return;
    }
    
    // Limit the total number of anchor points
    const int MAX_ANCHORS = 500; // Adjust based on your needs
    
    YsVec3 centerPos = mesh->GetVertexPosition(centerVtHd);
    int centerVtxId = mesh->GetSearchKey(centerVtHd);
    
    // Find one-ring neighborhood first
    std::unordered_set<int> excludeRegion;
    excludeRegion.insert(centerVtxId);
    
    auto neighbors = mesh->GetConnectedVertex(centerVtHd);
    for (const auto& nbVtHd : neighbors) {
        excludeRegion.insert(mesh->GetSearchKey(nbVtHd));
    }
    
    // For large meshes, sample vertices instead of checking all
    int totalVertices = mesh->GetNumVertices();
    int samplingRate = totalVertices / MAX_ANCHORS;
    samplingRate = std::max(1, samplingRate); // Ensure at least 1
    
    // Create a limited number of anchors
    int count = 0;
    int vertexIndex = 0;
    
    for (auto vtHd = mesh->FirstVertex(); vtHd != mesh->NullVertex() && count < MAX_ANCHORS; mesh->MoveToNext(vtHd)) {
        vertexIndex++;
        
        // Skip vertices if sampling rate > 1
        if (samplingRate > 1 && vertexIndex % samplingRate != 0) {
            continue;
        }
        
        int vtxId = mesh->GetSearchKey(vtHd);
        
        // Skip vertices in the exclusion region
        if (excludeRegion.count(vtxId) > 0) {
            continue;
        }
        
        // Skip distance calculation if we're sampling
        if (samplingRate > 1) {
            anchor_vertices.insert(vtxId);
            count++;
            continue;
        }
        
        // Only perform distance calculation if needed
        YsVec3 pos = mesh->GetVertexPosition(vtHd);
        YsVec3 diff = pos - centerPos;
        double distance = sqrt(diff.x()*diff.x() + diff.y()*diff.y() + diff.z()*diff.z());
        
        if (distance > radius) {
            anchor_vertices.insert(vtxId);
            count++;
        }
    }
    
    cout << "Created " << anchor_vertices.size() << " anchor points" << endl;
}

void LaplacianEditor::clearSelection() {
    selected_vertices.clear();
    control_points.clear();
    anchor_vertices.clear();
    cout << "Selection cleared" << endl;
}

void LaplacianEditor::moveSelected(const YsVec3& delta) {
    if (selected_vertices.empty()) return;
    
    for (int vtx_id : selected_vertices) {
        YsVec3 current_pos = control_points[vtx_id];
        YsVec3 new_pos;
        new_pos.SetX(current_pos.x() + delta.x());
        new_pos.SetY(current_pos.y() + delta.y());
        new_pos.SetZ(current_pos.z() + delta.z());
        control_points[vtx_id] = new_pos;
    }
    
    cout << "Moved selected vertices by (" << delta.x() << ", " << delta.y() << ", " << delta.z() << ")" << endl;
}

void LaplacianEditor::moveSelectedX(double delta) {
    YsVec3 move_delta;
    move_delta.SetX(delta);
    move_delta.SetY(0);
    move_delta.SetZ(0);
    moveSelected(move_delta);
}

void LaplacianEditor::moveSelectedY(double delta) {
    YsVec3 move_delta;
    move_delta.SetX(0);
    move_delta.SetY(delta);
    move_delta.SetZ(0);
    moveSelected(move_delta);
}

void LaplacianEditor::moveSelectedZ(double delta) {
    YsVec3 move_delta;
    move_delta.SetX(0);
    move_delta.SetY(0);
    move_delta.SetZ(delta);
    moveSelected(move_delta);
}

void LaplacianEditor::applyDeformation(double lambda) {
    if (control_points.empty()) {
        cout << "No control points defined, skipping deformation" << endl;
        return;
    }
    
    applyLaplacianDeformation(*mesh, control_points, anchor_vertices, lambda);
    
    // After deformation, update the control point positions to reflect their new locations
    for (auto& [vtx_id, _] : control_points) {
        auto vtHd = findVertexById(vtx_id);
        if (vtHd != mesh->NullVertex()) {
            control_points[vtx_id] = mesh->GetVertexPosition(vtHd);
        }
    }
}

bool LaplacianEditor::handleKeyInput(int key) {
    if (selected_vertices.empty()) return false;
    
    bool handled = true;
    
    switch (key) {
        // X-axis movement
        case 'x': moveSelectedX(-movement_step); break;
        case 'X': moveSelectedX(movement_step); break;
        
        // Y-axis movement
        case 'y': moveSelectedY(-movement_step); break;
        case 'Y': moveSelectedY(movement_step); break;
        
        // Z-axis movement
        case 'z': moveSelectedZ(-movement_step); break;
        case 'Z': moveSelectedZ(movement_step); break;
        
        // Apply deformation
        case 'a': case 'A': applyDeformation(); break;
        
        // Reset to original
        case 'r': case 'R': resetToOriginal(); break;
        
        // Clear selection
        case 'c': case 'C': clearSelection(); break;
        
        default: handled = false;
    }
    
    return handled;
}

void LaplacianEditor::storeOriginalPositions() {
    original_positions.clear();
    for (auto vtHd = mesh->FirstVertex(); vtHd != mesh->NullVertex(); mesh->MoveToNext(vtHd)) {
        int vtx_id = mesh->GetSearchKey(vtHd);
        original_positions[vtx_id] = mesh->GetVertexPosition(vtHd);
    }
    cout << "Stored original positions for " << original_positions.size() << " vertices" << endl;
}

void LaplacianEditor::resetToOriginal() {
    for (const auto& [vtx_id, pos] : original_positions) {
        auto vtHd = findVertexById(vtx_id);
        if (vtHd != mesh->NullVertex()) {
            mesh->SetVertexPosition(vtHd, pos);
            if (control_points.count(vtx_id)) {
                control_points[vtx_id] = pos;
            }
        }
    }
    cout << "Reset to original positions" << endl;
}
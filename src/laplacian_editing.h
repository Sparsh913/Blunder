// laplacian_editing.h
#pragma once
#include <iostream>
#include "mesh.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>

// Type alias - Mesh in Laplacian editing refers to PolygonalMesh
typedef PolygonalMesh Mesh;

// The original Laplacian deformation function
void applyLaplacianDeformation(Mesh &mesh,
    const std::unordered_map<int, YsVec3> &control_points,
    const std::unordered_set<int> &anchor_points,
    double lambda = 100.0);

// Class to manage Laplacian editing operations
class LaplacianEditor {
public:
    LaplacianEditor(Mesh* mesh);
    ~LaplacianEditor() = default;
    
    // Selection management - supports both handle and position selection
    bool selectVertex(const YsVec3& position, double threshold = 0.1);
    void clearSelection();
    bool hasSelection() const { return !selected_vertices.empty(); }
    
    // Create anchor points to prevent global movement
    void createAnchors(const Mesh::VertexHandle& centerVtHd, double radius);
    const std::unordered_set<int>& getAnchorVertices() const { return anchor_vertices; }
    
    // Movement and deformation
    void moveSelected(const YsVec3& delta);
    void moveSelectedX(double delta);
    void moveSelectedY(double delta);
    void moveSelectedZ(double delta);
    void applyDeformation(double lambda = 100.0);
    
    // Keyboard input handling
    bool handleKeyInput(int key);
    
    // Get selected vertices
    const std::unordered_set<int>& getSelectedVertices() const { return selected_vertices; }
    
    // Undo functionality
    void storeOriginalPositions();
    void resetToOriginal();
    
private:
    // Cache mapping from vertex IDs to handles
    void cacheVertexHandles();
    Mesh::VertexHandle findVertexById(int vtxId) const;
    
    Mesh* mesh;
    std::unordered_set<int> selected_vertices;
    std::unordered_set<int> anchor_vertices;  // Vertices that are fixed during deformation
    std::unordered_map<int, YsVec3> control_points;
    std::unordered_map<int, YsVec3> original_positions;
    std::unordered_map<int, Mesh::VertexHandle> vtxIdToHandle; // Cache for faster lookups
    double movement_step = 0.1;
};
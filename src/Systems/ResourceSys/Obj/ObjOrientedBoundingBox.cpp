#include "ObjOrientedBoundingBox.hpp"

#include <Eigen/Dense>
#include <limits>

#include "ObjResource.hpp"

namespace {
// Get points from vertices with mesh transformations applied
std::vector<glm::vec3> getModelspacePoints(const ObjResource& resource) {
    std::vector<glm::vec3> points;

    for(const auto& mesh : resource.objMeshes) {
        for(const auto& index : mesh->indices) {
            const auto& vertex = resource.vertices[index];
            points.push_back(
                glm::vec3(mesh->transform * glm::vec4(vertex.position, 1.0f)));
        }
    }
    return points;
}

Eigen::Matrix3f computeCovarianceMatrix(const std::vector<glm::vec3>& points,
                                  Eigen::Vector3f& centroid) {
    centroid = Eigen::Vector3f::Zero();
    for(const auto& p : points) {
        centroid += Eigen::Vector3f(p.x, p.y, p.z);
    }
    centroid /= static_cast<float>(points.size());

    Eigen::Matrix3f covariance = Eigen::Matrix3f::Zero();
    for (const auto& p : points) {
        auto&& point = Eigen::Vector3f(p.x, p.y, p.z);
        Eigen::Vector3f diff = point - centroid;
        covariance += diff * diff.transpose();
    }
    return covariance /= static_cast<float>(points.size());
}

// Compute eigenvalues and eigenvectors using Eigen library
void computeEigenDecomposition(const Eigen::Matrix3f& matrix,
                               Eigen::Vector3f& eigenvalues, Eigen::Matrix3f& eigenvectors) {
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(matrix);
    eigenvalues = solver.eigenvalues();
    eigenvectors = solver.eigenvectors();
}
} // namespace

ObjOrientedBoundingBox::ObjOrientedBoundingBox(const ObjResource& resource) {
    calculateCorners(resource);
}

std::pair<glm::vec3, glm::vec3> ObjOrientedBoundingBox::getWorldspaceAABB(
    const glm::mat4& modelMatrix) const {
    glm::vec3 outMinCorner = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 outMaxCorner = glm::vec3(std::numeric_limits<float>::lowest());

    // Define the 8 corners of the OBB
    glm::vec3 corners[8] = {minCorner,
                            {maxCorner.x, minCorner.y, minCorner.z},
                            {maxCorner.x, maxCorner.y, minCorner.z},
                            {minCorner.x, maxCorner.y, minCorner.z},
                            {minCorner.x, minCorner.y, maxCorner.z},
                            {maxCorner.x, minCorner.y, maxCorner.z},
                            {maxCorner.x, maxCorner.y, maxCorner.z},
                            {minCorner.x, maxCorner.y, maxCorner.z}};

    // Transform each corner and compute AABB min/max
    for(const auto& corner : corners) {
        glm::vec3 transformedCorner = glm::vec3(modelMatrix * glm::vec4(corner, 1.0f));
        outMinCorner = glm::min(outMinCorner, transformedCorner);
        outMaxCorner = glm::max(outMaxCorner, transformedCorner);
    }

    return {outMinCorner, outMaxCorner};
}

void ObjOrientedBoundingBox::calculateCorners(const ObjResource& resource) {
    std::vector<glm::vec3> points = getModelspacePoints(resource);

    Eigen::Vector3f centroid;
    Eigen::Matrix3f covariance = computeCovarianceMatrix(points, centroid);

    Eigen::Vector3f eigenvalues;
    Eigen::Matrix3f eigenvectors;
    computeEigenDecomposition(covariance, eigenvalues, eigenvectors);

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(covariance);
    Eigen::Matrix3f eigenVectors = solver.eigenvectors();
    Eigen::Vector3f eigenValues = solver.eigenvalues();

    // Step 4: Project the points onto the principal axes
    Eigen::Matrix3f rotation = eigenVectors;
    Eigen::Vector3f minExtents =
        Eigen::Vector3f::Ones() * std::numeric_limits<float>::max();
    Eigen::Vector3f maxExtents =
        Eigen::Vector3f::Ones() * std::numeric_limits<float>::lowest();

    for(const auto& point : points) {
        Eigen::Vector3f projected =
            rotation.transpose() *
            (Eigen::Vector3f(point.x, point.y, point.z) - centroid);
        minExtents = minExtents.cwiseMin(projected);
        maxExtents = maxExtents.cwiseMax(projected);
    }

    // Step 5: Calculate the min and max corners of the OBB
    auto eigenMinCorner = rotation * minExtents + centroid;
    auto eigenMaxCorner = rotation * maxExtents + centroid;
    minCorner = glm::vec3(eigenMinCorner.x(), eigenMinCorner.y(), eigenMinCorner.z());
    maxCorner = glm::vec3(eigenMaxCorner.x(), eigenMaxCorner.y(), eigenMaxCorner.z());
}

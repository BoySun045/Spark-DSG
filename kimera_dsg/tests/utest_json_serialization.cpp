#include "kimera_dsg_tests/temp_file.h"

#include <kimera_dsg/dynamic_scene_graph.h>

#include <gtest/gtest.h>

namespace kimera {

TEST(SceneGraphSerializationTests, SerializeDsgBasic) {
  DynamicSceneGraph expected({1, 2, 3}, 0);
  expected.emplaceNode(1, 0, std::make_unique<NodeAttributes>());
  expected.emplaceNode(1, 1, std::make_unique<NodeAttributes>());
  expected.emplaceNode(3, 2, std::make_unique<NodeAttributes>());

  expected.insertEdge(0, 1);
  expected.insertEdge(1, 2);

  const auto output = expected.serialize();

  auto result = DynamicSceneGraph::deserialize(output);

  EXPECT_EQ(expected.numNodes(), result->numNodes()) << output;
  EXPECT_EQ(expected.numEdges(), result->numEdges());
  EXPECT_EQ(expected.numLayers(), result->numLayers());
  EXPECT_EQ(expected.layer_ids, result->layer_ids);

  EXPECT_TRUE(result->hasNode(0));
  EXPECT_TRUE(result->hasNode(1));
  EXPECT_TRUE(result->hasNode(2));
  EXPECT_TRUE(result->hasEdge(0, 1));
  EXPECT_TRUE(result->hasEdge(1, 2));
  EXPECT_EQ(expected.hasLayer(0), result->hasLayer(0));
}

TEST(SceneGraphSerializationTests, SerializeDsgWithNaNs) {
  DynamicSceneGraph expected({1, 2, 3}, 0);
  expected.emplaceNode(1, 0, std::make_unique<NodeAttributes>());
  expected.emplaceNode(1, 1, std::make_unique<NodeAttributes>());
  expected.emplaceNode(3, 2, std::make_unique<NodeAttributes>());
  Eigen::Vector3d bad_pos = Eigen::Vector3d::Zero();
  bad_pos(0) = std::numeric_limits<double>::quiet_NaN();
  bad_pos(1) = std::numeric_limits<double>::quiet_NaN();
  bad_pos(2) = std::numeric_limits<double>::quiet_NaN();
  expected.emplaceNode(3, 3, std::make_unique<NodeAttributes>(bad_pos));

  expected.insertEdge(0, 1);
  expected.insertEdge(1, 2);
  expected.insertEdge(2, 3);

  const std::string output_str = expected.serialize();

  auto result = DynamicSceneGraph::deserialize(output_str);

  EXPECT_EQ(expected.numNodes(), result->numNodes());
  EXPECT_EQ(expected.numEdges(), result->numEdges());
  EXPECT_EQ(expected.numLayers(), result->numLayers());
  EXPECT_EQ(expected.layer_ids, result->layer_ids);

  EXPECT_TRUE(result->hasNode(0));
  EXPECT_TRUE(result->hasNode(1));
  EXPECT_TRUE(result->hasNode(2));
  EXPECT_TRUE(result->hasNode(3));
  EXPECT_TRUE(result->hasEdge(0, 1));
  EXPECT_TRUE(result->hasEdge(1, 2));
  EXPECT_TRUE(result->hasEdge(2, 3));
  EXPECT_EQ(expected.hasLayer(0), result->hasLayer(0));
}

TEST(SceneGraphSerializationTests, SerializeDsgDynamic) {
  using namespace std::chrono_literals;
  DynamicSceneGraph expected;
  expected.emplaceNode(3, 0, std::make_unique<NodeAttributes>());

  expected.emplaceNode(2, 'a', 10ns, std::make_unique<NodeAttributes>());
  expected.emplaceNode(2, 'a', 20ns, std::make_unique<NodeAttributes>());
  expected.emplaceNode(2, 'a', 30ns, std::make_unique<NodeAttributes>(), false);
  expected.emplaceNode(2, 'a', 40ns, std::make_unique<NodeAttributes>());

  const auto output = expected.serialize();

  auto result = DynamicSceneGraph::deserialize(output);

  EXPECT_EQ(expected.numNodes(), result->numNodes()) << output;
  EXPECT_EQ(expected.numEdges(), result->numEdges());
  EXPECT_EQ(expected.numLayers(), result->numLayers());
  EXPECT_EQ(expected.layer_ids, result->layer_ids);

  EXPECT_TRUE(result->hasNode(0));
  EXPECT_TRUE(result->hasNode(NodeSymbol('a', 0)));
  EXPECT_TRUE(result->hasNode(NodeSymbol('a', 1)));
  EXPECT_TRUE(result->hasNode(NodeSymbol('a', 2)));
  EXPECT_TRUE(result->hasNode(NodeSymbol('a', 3)));
  EXPECT_TRUE(result->hasEdge(NodeSymbol('a', 0), NodeSymbol('a', 1)));
  EXPECT_FALSE(result->hasEdge(NodeSymbol('a', 1), NodeSymbol('a', 2)));
  EXPECT_TRUE(result->hasEdge(NodeSymbol('a', 2), NodeSymbol('a', 3)));

  EXPECT_TRUE(result->hasLayer(2, 'a'));
}

TEST(SceneGraphSerializationTests, SaveAndLoadGraph) {
  TempFile tmp_file;

  DynamicSceneGraph graph;
  graph.emplaceNode(KimeraDsgLayers::PLACES,
                    NodeSymbol('p', 0),
                    std::make_unique<NodeAttributes>(Eigen::Vector3d::Zero()));

  DynamicSceneGraph::MeshVertices fake_vertices;
  pcl::PolygonMesh fake_mesh;
  pcl::toPCLPointCloud2(fake_vertices, fake_mesh.cloud);
  graph.setMeshDirectly(fake_mesh);

  graph.save(tmp_file.path);

  auto other = DynamicSceneGraph::load(tmp_file.path);

  EXPECT_EQ(graph.numNodes(), other->numNodes());
  EXPECT_EQ(graph.numLayers(), other->numLayers());
  EXPECT_EQ(graph.hasMesh(), other->hasMesh());
}

}  // namespace kimera
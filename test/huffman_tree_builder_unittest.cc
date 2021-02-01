#include <string>
#include <memory>

#include "encryption/huffman_tree/huffman_tree_builder.h"
#include "letter/one_byte_letter.h"
#include "gtest/gtest.h"

namespace huffman_tree {

class HuffmanTreeBuilderTest : public ::testing::Test {
 public:
  std::unique_ptr<TreeNode> BuildTree(const std::string& text) {
    auto string_input = std::make_shared<std::istringstream>(text);
    auto letterLexer = std::make_shared<letter::OneByteLetterLexer>();
    return BuildHuffmanTree(letterLexer->Split(std::move(string_input)));
  }
};

TEST_F(HuffmanTreeBuilderTest, OneSymbol) {
  const std::string kTestString = "aaaa";
  const auto root = BuildTree(kTestString);

  ASSERT_TRUE(root);
  EXPECT_TRUE(root->isLeaf());
  EXPECT_EQ(root->key_->toString(), "a");
  EXPECT_EQ(root->frequency_, 4u);
}

TEST_F(HuffmanTreeBuilderTest, TwoSymbols) {
  const std::string kTestString = "aaaabbb";
  const auto root = BuildTree(kTestString);

  ASSERT_TRUE(root);
  EXPECT_TRUE(root->isInner());
  EXPECT_EQ(root->frequency_, 7u);

  auto* left = root->left_.get();
  EXPECT_EQ(left->key_->toString(), "b");
  EXPECT_EQ(left->frequency_, 3u);
  EXPECT_TRUE(left->isLeaf());

  auto* right = root->right_.get();
  EXPECT_EQ(right->key_->toString(), "a");
  EXPECT_EQ(right->frequency_, 4u);
  EXPECT_TRUE(right->isLeaf());
}

TEST_F(HuffmanTreeBuilderTest, ThreeSymbols) {
  const std::string kTestString = "aaaabbc";
  const auto root = BuildTree(kTestString);

  ASSERT_TRUE(root);
  EXPECT_TRUE(root->isInner());
  EXPECT_EQ(root->frequency_, 7u);

  auto* left = root->left_.get();
  EXPECT_TRUE(left->isInner());
  EXPECT_EQ(left->frequency_, 3u);

  auto* right = root->right_.get();
  EXPECT_EQ(right->key_->toString(), "a");
  EXPECT_EQ(right->frequency_, 4u);
  EXPECT_TRUE(right->isLeaf());

  auto* left_left = left->left_.get();
  EXPECT_TRUE(left_left->isLeaf());
  EXPECT_EQ(left_left->key_->toString(), "c");
  EXPECT_EQ(left_left->frequency_, 1u);

  auto* left_right = left->right_.get();
  EXPECT_TRUE(left_right->isLeaf());
  EXPECT_EQ(left_right->key_->toString(), "b");
  EXPECT_EQ(left_right->frequency_, 2u);
}

}  // namespace huffman_tree

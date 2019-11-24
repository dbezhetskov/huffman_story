#include "encryption/huffman_encryption.h"

#include <cassert>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "bits_manipulation/bits_manipulation.h"

namespace encryption {

namespace {

constexpr bool kInnerNodeBitLabel = false;
constexpr bool kLeafNodeBitLabel = true;

constexpr bool kTurnLeftBitLabel = false;
constexpr bool kTurnRightBitLabel = true;

std::unordered_map<char, std::vector<bool>> BuildCodesMap(TreeNode* root);
bool IsInnerNode(encryption::TreeNode* node);
bool IsLeafNode(encryption::TreeNode* node);
std::string ReadBytesToString(std::unique_ptr<BitReader> byte_reader);

}  // namespace

void HuffmanEncryption::Encrypt(std::unique_ptr<BitReader> input,
                                std::unique_ptr<BitWriter> output) {
  output_ = std::move(output);
  const auto text = ReadBytesToString(std::move(input));
  auto root = HuffmanTreeBuilder(text).GetRoot();
  WriteTreeInPrefixForm(root.get());
  WriteEncryptedText(root.get(), text);
}

namespace {
std::string ReadBytesToString(std::unique_ptr<BitReader> byte_reader) {
  std::string result;
  for (auto byte = byte_reader->ReadByte(); byte.has_value();
       byte = byte_reader->ReadByte()) {
    result.push_back(*byte);
  }

  return result;
}
}  // namespace

void HuffmanEncryption::WriteTreeInPrefixForm(TreeNode* root) {
  if (!root) {
    return;
  }

  const auto node_label =
      IsInnerNode(root) ? kInnerNodeBitLabel : kLeafNodeBitLabel;
  output_->WriteBit(node_label);
  if (node_label == kLeafNodeBitLabel) {
    output_->WriteByte(root->key_.back());
  }

  WriteTreeInPrefixForm(root->left_.get());
  WriteTreeInPrefixForm(root->right_.get());
}

namespace {
bool IsInnerNode(encryption::TreeNode* node) {
  return node->left_ && node->right_;
}
}  // namespace

void HuffmanEncryption::WriteEncryptedText(TreeNode* root,
                                           std::string_view text) {
  const auto codes_by_symbol = BuildCodesMap(root);

  for (const auto symbol : text) {
    const auto code_iter = codes_by_symbol.find(symbol);
    assert(code_iter != codes_by_symbol.end());
    for (const auto bit : code_iter->second) {
      output_->WriteBit(bit);
    }
  }
}

namespace {
std::unordered_map<char, std::vector<bool>> BuildCodesMap(TreeNode* root) {
  std::unordered_map<char, std::vector<bool>> codes;

  if (root && !root->left_ && !root->right_) {
    codes[root->key_.back()] = std::vector<bool>(1, kTurnLeftBitLabel);
    return codes;
  }

  struct NodeWithCode {
    TreeNode* node;
    std::vector<bool> code;
  };

  std::unordered_set<const TreeNode*> used;
  std::stack<NodeWithCode> stack;

  stack.push(NodeWithCode{root, std::vector<bool>()});

  while (!stack.empty()) {
    const auto current = stack.top();
    const auto* current_node = current.node;
    stack.pop();

    if (!current_node) {
      continue;
    }

    if (!current_node->left_ && !current_node->right_) {
      codes[current_node->key_.back()] = current.code;
      continue;
    }

    if (current_node->left_ &&
        used.find(current_node->left_.get()) == used.end()) {
      auto code = current.code;
      code.push_back(kTurnLeftBitLabel);
      stack.push(NodeWithCode{current_node->left_.get(), std::move(code)});
    }

    if (current_node->right_ &&
        used.find(current_node->right_.get()) == used.end()) {
      auto code = current.code;
      code.push_back(kTurnRightBitLabel);
      stack.push(NodeWithCode{current_node->right_.get(), std::move(code)});
    }

    used.insert(current_node);
  }

  return codes;
}
}  // namespace

void HuffmanEncryption::Decrypt(std::unique_ptr<BitReader> input,
                                std::unique_ptr<BitWriter> output) {
  input_ = std::move(input);
  output_ = std::move(output);
  auto root = ReadTreeInPrefixForm();
  DecryptText(root.get());
}

std::unique_ptr<TreeNode> HuffmanEncryption::ReadTreeInPrefixForm() {
  const auto bit = input_->ReadBit();
  if (!bit.has_value()) {
    return nullptr;
  }

  if (*bit == kLeafNodeBitLabel) {
    const auto symbol = input_->ReadByte();
    assert(symbol.has_value());
    return std::make_unique<TreeNode>(std::string(1, *symbol), 0, nullptr,
                                      nullptr);
  }

  assert(*bit == kInnerNodeBitLabel);
  auto node = std::make_unique<TreeNode>("", 0, nullptr, nullptr);
  node->left_ = std::move(ReadTreeInPrefixForm());
  node->right_ = std::move(ReadTreeInPrefixForm());
  return node;
}

void HuffmanEncryption::DecryptText(TreeNode* root) {
  if (!root) {
    return;
  }

  auto* current_node = root;
  for (auto bit = input_->ReadBit(); bit.has_value(); bit = input_->ReadBit()) {
    if (IsLeafNode(current_node)) {
      output_->WriteByte(current_node->key_.back());
      continue;
    }

    if (*bit == kTurnLeftBitLabel) {
      current_node = current_node->left_.get();
      continue;
    }

    assert(*bit == kTurnRightBitLabel);
    current_node = current_node->right_.get();
  }
}

namespace {
bool IsLeafNode(encryption::TreeNode* node) {
  return !node->left_ && !node->right_;
}
}  // namespace

}  // namespace encryption

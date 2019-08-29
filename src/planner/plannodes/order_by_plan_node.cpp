#include "planner/plannodes/order_by_plan_node.h"
#include <utility>
#include <vector>

namespace terrier::planner {

common::hash_t OrderByPlanNode::Hash() const {
  common::hash_t hash = AbstractPlanNode::Hash();

  // Sort Keys
  for (const auto &sort_key : sort_keys_) {
    hash = common::HashUtil::CombineHashes(hash, sort_key.first->Hash());
    hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(sort_key.second));
  }

  // Inlined Limit Stuff
  hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(has_limit_));
  if (has_limit_) {
    hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(limit_));
    hash = common::HashUtil::CombineHashes(hash, common::HashUtil::Hash(offset_));
  }

  return hash;
}

bool OrderByPlanNode::operator==(const AbstractPlanNode &rhs) const {
  if (!AbstractPlanNode::operator==(rhs)) return false;

  auto &other = static_cast<const OrderByPlanNode &>(rhs);

  // Sort Keys
  if (sort_keys_.size() != other.sort_keys_.size()) return false;
  for (auto i = 0U; i < sort_keys_.size(); i++) {
    auto &sort_key = sort_keys_[i];
    auto &other_sort_key = other.sort_keys_[i];
    if (sort_key.second != other_sort_key.second) return false;
    if (*sort_key.first != *other_sort_key.first) return false;
  }

  //  Inlined Limit Stuff
  if (has_limit_ != other.has_limit_) return false;
  if (has_limit_) {
    // Limit
    if (limit_ != other.limit_) return false;

    // Offset
    if (offset_ != other.offset_) return false;
  }

  return true;
}

nlohmann::json OrderByPlanNode::ToJson() const {
  nlohmann::json j = AbstractPlanNode::ToJson();
  j["sort_keys"] = sort_keys_;
  j["has_limit"] = has_limit_;
  j["limit"] = limit_;
  j["offset"] = offset_;
  return j;
}

void OrderByPlanNode::FromJson(const nlohmann::json &j) {
  AbstractPlanNode::FromJson(j);
  auto sort_keys = j.at("sort_keys").get<std::vector<std::pair<nlohmann::json, OrderByOrderingType>>>();
  for (auto &pair : sort_keys) {
    sort_keys_.emplace_back(parser::DeserializeExpression(pair.first), pair.second);
  }

  has_limit_ = j.at("has_limit").get<bool>();
  limit_ = j.at("limit").get<size_t>();
  offset_ = j.at("offset").get<size_t>();
}

}  // namespace terrier::planner

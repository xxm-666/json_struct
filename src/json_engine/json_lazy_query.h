#pragma once
#include "json_filter.h"
#include <stack>
#include <string>
#include <utility>
#include <optional>
#include <vector>

namespace JsonStruct
{

    class LazyJsonPathIterator
    {
    public:
        struct Frame
        {
            const JsonValue *value;
            std::string path;
            size_t nodeIndex;
            
            // 递归状态信息
            enum class RecursiveState { None, SearchingSelf, SearchingChildren };
            RecursiveState recursiveState = RecursiveState::None;
            std::string recursiveProperty; // 用于递归属性搜索
            
            // 子节点迭代状态
            std::vector<std::pair<std::string, const JsonValue*>> children; // 对象子节点
            size_t childIndex = 0; // 当前遍历的子节点索引
            
            // 用于数组遍历
            size_t arrayIndex = 0;
            size_t arraySize = 0;
            
            Frame(const JsonValue* v, const std::string& p, size_t ni) 
                : value(v), path(p), nodeIndex(ni) {}
        };

        LazyJsonPathIterator(const JsonValue *root, const std::vector<jsonpath::PathNode> &nodes)
            : nodes_(nodes)
        {
            stack_.push(Frame(root, "$", 0));
            advance();
        }

        bool hasNext() const { return current_.has_value(); }

        // 返回下一个匹配结果
        QueryResult next()
        {
            if (!current_)
                throw std::out_of_range("No more results");
            QueryResult result = *current_;
            advance();
            return result;
        }

    private:
        std::vector<jsonpath::PathNode> nodes_;
        std::stack<Frame> stack_;
        std::optional<QueryResult> current_;

        void advance()
        {
            current_.reset();
            
            while (!stack_.empty())
            {
                Frame& frame = stack_.top();

                // 如果已经走到最后一个节点，返回结果
                if (frame.nodeIndex == nodes_.size())
                {
                    current_ = QueryResult(frame.value, frame.path, frame.nodeIndex);
                    stack_.pop();
                    return;
                }

                const auto& node = nodes_[frame.nodeIndex];
                bool shouldContinue = processNode(frame, node);
                
                if (!shouldContinue)
                {
                    stack_.pop();
                }
                
                // 如果找到了结果，就返回
                if (current_.has_value())
                {
                    return;
                }
            }
        }

    private:
        bool processNode(Frame& frame, const jsonpath::PathNode& node)
        {
            switch (node.type)
            {
                case jsonpath::NodeType::ROOT:
                    frame.nodeIndex++;
                    return true;
                    
                case jsonpath::NodeType::PROPERTY:
                    return processProperty(frame, node.property);
                    
                case jsonpath::NodeType::INDEX:
                    return processIndex(frame, node.index);
                    
                case jsonpath::NodeType::SLICE:
                    return processSlice(frame, node.slice_start, node.slice_end);
                    
                case jsonpath::NodeType::WILDCARD:
                    return processWildcard(frame);
                    
                case jsonpath::NodeType::RECURSIVE:
                    return processRecursive(frame, node.property);
                    
                default:
                    return false;
            }
        }
        
        bool processProperty(Frame& frame, const std::string& property)
        {
            if (frame.value->isObject())
            {
                const auto& obj = frame.value->getObject();
                if (obj)
                {
                    auto it = obj->find(property);
                    if (it != obj->end())
                    {
                        // 更新当前frame指向找到的属性值
                        frame.value = &it->second;
                        frame.path += "." + property;
                        frame.nodeIndex++;
                        return true;
                    }
                }
            }
            return false; // 没找到，移除这个frame
        }
        
        bool processIndex(Frame& frame, int index)
        {
            if (frame.value->isArray())
            {
                const auto& arr = frame.value->getArray();
                if (arr && index >= 0 && static_cast<size_t>(index) < arr->size())
                {
                    frame.value = &(*arr)[index];
                    frame.path += "[" + std::to_string(index) + "]";
                    frame.nodeIndex++;
                    return true;
                }
            }
            return false;
        }
        
        bool processSlice(Frame& frame, int start, int end)
        {
            if (!frame.value->isArray()) return false;
            
            const auto& arr = frame.value->getArray();
            if (!arr) return false;
            
            // 初始化数组遍历状态
            if (frame.arraySize == 0)
            {
                int actualEnd = end == -1 ? static_cast<int>(arr->size()) : end;
                if (start < 0) start = 0;
                if (actualEnd > static_cast<int>(arr->size())) actualEnd = static_cast<int>(arr->size());
                
                frame.arrayIndex = start;
                frame.arraySize = actualEnd;
            }
            
            // 检查是否还有元素要处理
            if (frame.arrayIndex < frame.arraySize)
            {
                // 创建新的frame处理当前数组元素
                Frame newFrame(frame.value, frame.path, frame.nodeIndex);
                newFrame.value = &(*arr)[frame.arrayIndex];
                newFrame.path = frame.path + "[" + std::to_string(frame.arrayIndex) + "]";
                newFrame.nodeIndex = frame.nodeIndex + 1;
                
                frame.arrayIndex++; // 为下次迭代准备
                
                // 如果这是最后一个元素，标记当前frame为完成
                if (frame.arrayIndex >= frame.arraySize)
                {
                    stack_.pop(); // 移除当前frame
                }
                
                stack_.push(newFrame);
                return true;
            }
            
            return false;
        }
        
        bool processWildcard(Frame& frame)
        {
            // 初始化子节点列表
            if (frame.children.empty() && frame.childIndex == 0)
            {
                if (frame.value->isObject())
                {
                    const auto& obj = frame.value->getObject();
                    if (obj)
                    {
                        for (const auto& [key, child] : *obj)
                        {
                            frame.children.emplace_back(key, &child);
                        }
                    }
                }
                else if (frame.value->isArray())
                {
                    const auto& arr = frame.value->getArray();
                    if (arr)
                    {
                        for (size_t i = 0; i < arr->size(); ++i)
                        {
                            frame.children.emplace_back("[" + std::to_string(i) + "]", &(*arr)[i]);
                        }
                    }
                }
            }
            
            // 检查是否还有子节点要处理
            if (frame.childIndex < frame.children.size())
            {
                const auto& [childPath, childValue] = frame.children[frame.childIndex];
                
                // 创建新的frame处理当前子节点
                Frame newFrame(childValue, frame.path + (childPath[0] == '[' ? childPath : "." + childPath), frame.nodeIndex + 1);
                
                frame.childIndex++; // 为下次迭代准备
                
                // 如果这是最后一个子节点，标记当前frame为完成
                if (frame.childIndex >= frame.children.size())
                {
                    stack_.pop(); // 移除当前frame
                }
                
                stack_.push(newFrame);
                return true;
            }
            
            return false;
        }
        
        bool processRecursive(Frame& frame, const std::string& property)
        {
            // 递归搜索的状态机
            switch (frame.recursiveState)
            {
                case Frame::RecursiveState::None:
                    // 初始化递归搜索
                    frame.recursiveState = Frame::RecursiveState::SearchingSelf;
                    frame.recursiveProperty = property;
                    return processRecursive(frame, property); // 立即进入下一状态
                    
                case Frame::RecursiveState::SearchingSelf:
                    // 检查当前节点是否匹配
                    if (!property.empty() && frame.value->isObject())
                    {
                        const auto& obj = frame.value->getObject();
                        if (obj)
                        {
                            auto it = obj->find(property);
                            if (it != obj->end())
                            {
                                // 找到匹配，创建结果frame
                                Frame resultFrame(&it->second, frame.path + "." + property, frame.nodeIndex + 1);
                                stack_.push(resultFrame);
                                
                                // 转换到搜索子节点状态
                                frame.recursiveState = Frame::RecursiveState::SearchingChildren;
                                return true;
                            }
                        }
                    }
                    // 没找到匹配或者是通用递归，直接转到搜索子节点
                    frame.recursiveState = Frame::RecursiveState::SearchingChildren;
                    return processRecursive(frame, property);
                    
                case Frame::RecursiveState::SearchingChildren:
                    // 初始化子节点列表（如果还没有初始化）
                    if (frame.children.empty() && frame.childIndex == 0)
                    {
                        if (frame.value->isObject())
                        {
                            const auto& obj = frame.value->getObject();
                            if (obj)
                            {
                                for (const auto& [key, child] : *obj)
                                {
                                    frame.children.emplace_back(key, &child);
                                }
                            }
                        }
                        else if (frame.value->isArray())
                        {
                            const auto& arr = frame.value->getArray();
                            if (arr)
                            {
                                for (size_t i = 0; i < arr->size(); ++i)
                                {
                                    frame.children.emplace_back("[" + std::to_string(i) + "]", &(*arr)[i]);
                                }
                            }
                        }
                    }
                    
                    // 处理下一个子节点
                    if (frame.childIndex < frame.children.size())
                    {
                        const auto& [childPath, childValue] = frame.children[frame.childIndex];
                        
                        // 创建递归frame处理子节点
                        Frame recursiveFrame(childValue, frame.path + (childPath[0] == '[' ? childPath : "." + childPath), frame.nodeIndex);
                        recursiveFrame.recursiveState = Frame::RecursiveState::None; // 重新开始递归搜索
                        
                        frame.childIndex++;
                        
                        // 如果这是最后一个子节点，移除当前frame
                        if (frame.childIndex >= frame.children.size())
                        {
                            stack_.pop();
                        }
                        
                        stack_.push(recursiveFrame);
                        return true;
                    }
                    
                    return false; // 没有更多子节点
            }
            
            return false;
        }
    };

} // namespace JsonStruct
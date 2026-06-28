#ifndef __PROGTEST__

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <climits>
#include <cfloat>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <variant>
#include <optional>
#include <compare>
#include <charconv>
#include <span>
#include <utility>
#include "expression.h"


using namespace std::literals;
using CValue = std::variant<std::monostate, double, std::string>;

constexpr unsigned SPREADSHEET_CYCLIC_DEPS = 0x01;
constexpr unsigned SPREADSHEET_FUNCTIONS = 0x02;
constexpr unsigned SPREADSHEET_FILE_IO = 0x04;
constexpr unsigned SPREADSHEET_SPEED = 0x08;
constexpr unsigned SPREADSHEET_PARSER = 0x10;
#endif /* __PROGTEST__ */

#define SEPARATORPOSVAL ";:?~$~?:;"
#define SEPARATORVALPOS "?:?;$;?:?"
#define END_FINITE_STATE_MACHINE if(i >= original.size())\
{\
state = 4;\
break;\
}

class CPos;
class CSpreadsheet;
class CAst;
class CCell;
class CNode;
class CExprBuilder;
class CMyExprBuilder;
class CRange;

bool leftColIsBigger(const std::string &lhs, const std::string &rhs)
{
    if(lhs.size() != rhs.size())
        return lhs.size() > rhs.size();

    return lhs > rhs;
}

class CAst
{
public:
    std::shared_ptr<std::map<CPos, CCell>> m_cells;
    std::shared_ptr<CNode> m_root;
    std::vector<std::shared_ptr<CNode>> m_currentLayer;
};

class CPos
{
public:
    int m_row1 = 0;
    std::string m_col1;
    bool m_lockRow1 = false;
    bool m_lockCol1 = false;

    CPos(std::string_view str)
    {
        if(str.size() < 2)
            throw std::invalid_argument("Invalid position");

        uint i  = 0;

        if(str[i] == '$')
        {
            m_lockCol1 = true;
            i++;
        }

        for(; i < str.size(); i++)
        {
            if(!isalpha(str[i]))
                break;
            m_col1 += std::toupper(str[i]);
        }

        if(str[i] == '$')
        {
            m_lockRow1 = true;
            i++;
        }

        bool isNumber = false;
        for(; i < str.size(); i++)
        {
            if(!isdigit(str[i]))
                throw std::invalid_argument("Invalid position");

            isNumber = true;
            int tmp = m_row1 * 10 + (str[i] - '0');
            m_row1 = tmp;
        }

        if(!isNumber || m_col1.empty())
            throw std::invalid_argument("Invalid position");
    }
    
    std::string toString() const
    {
        std::string res;
        if(m_lockCol1)
            res += '$';

        res += m_col1;

        if(m_lockRow1)
            res += '$';

        res += std::to_string(m_row1);
        return res;
    }

    // if left > right - return positive number
    // 26 letter
    static int difference(std::string left, std::string right)
    {
        int leftInt = 0;
        int rightInt = 0;

        for(char i : left)
        {
            leftInt = leftInt * 26 + (i - 'A' + 1);
        }
        for(char i : right)
        {
            rightInt = rightInt * 26 + (i - 'A' + 1);
        }

        return (leftInt - rightInt);
    }

    void incrementCol()
    {
        for(int i = (int)m_col1.size() - 1; i >= 0; i--)
        {
            if(m_col1[i] == 'Z')
            {
                m_col1[i] = 'A';
                if(i == 0)
                    m_col1 = 'A' + m_col1;
            }
            else
            {
                m_col1[i]++;
                break;
            }
        }
    }

    bool operator==(const CPos &rhs) const
    {
        return m_row1 == rhs.m_row1 && m_col1 == rhs.m_col1;
    }

    bool operator!=(const CPos &rhs) const
    {
        return !(rhs == *this);
    }

    bool operator<(const CPos &rhs) const
    {
        if (m_row1 < rhs.m_row1)
            return true;
        if (rhs.m_row1 < m_row1)
            return false;
        return m_col1 < rhs.m_col1;
    }

    bool operator>(const CPos &rhs) const
    {
        return rhs < *this;
    }

    bool operator<=(const CPos &rhs) const
    {
        return !(rhs < *this);
    }

    bool operator>=(const CPos &rhs) const
    {
        return !(*this < rhs);
    }

    void decrementCol() {
        if(m_col1.empty())
            throw std::invalid_argument("Invalid position");

        if(m_col1.size() == 1 && m_col1[0] == 'A')
            throw std::invalid_argument("Invalid position");

        for(int i = (int)m_col1.size() - 1; i >= 0; i--)
        {
            if(m_col1[i] == 'A')
            {
                m_col1[i] = 'Z';
                if(i == 0)
                    m_col1 = 'Z' + m_col1;
            }
            else
            {
                m_col1[i]--;
                break;
            }
        }

    }

private:

};

class CNode
{
public:
    std::shared_ptr<std::map<CPos, CCell>> m_cells;

    CNode() = default;
    CNode(std::shared_ptr<std::map<CPos, CCell>> cells) : m_cells(cells) {}
    virtual ~CNode() = default;
    virtual CValue eval() = 0;
    virtual bool tryCyclicChain(std::set<CPos>) = 0;
};

class CRange : public CNode
{
private:
    CPos m_tmp;

public:
    CRange(CPos start, CPos end) : m_tmp(start), m_start(start), m_end(end), m_leftUp(start), m_rightDown(end)
    {
        if(!leftColIsBigger(m_start.m_col1, m_end.m_col1))
        {
            m_leftUp.m_col1 = m_start.m_col1;
            m_rightDown.m_col1 = m_end.m_col1;
        }
        else
        {
            m_leftUp.m_col1 = m_end.m_col1;
            m_rightDown.m_col1 = m_start.m_col1;
        }

        m_leftUp.m_row1 = std::min(m_start.m_row1, m_end.m_row1);
        m_rightDown.m_row1 = std::max(m_start.m_row1, m_end.m_row1);

        m_tmp = m_leftUp;
    }

    bool tryCyclicChain(std::set<CPos> visited) override
    {
        // todo
        return false;
    }

    CValue eval() override
    {
        return {};
    }

    CPos increment()
    {

        if(m_tmp.m_col1 == m_rightDown.m_col1)
        {
            m_tmp.m_row1++;
            m_tmp.m_col1 = m_leftUp.m_col1;
        }
        else
            m_tmp.incrementCol();

        if(m_tmp.m_row1 > m_rightDown.m_row1)
            return m_rightDown;

        return m_tmp;
    }

    bool isEnd()
    {
        return m_tmp == m_rightDown;
    }

    CPos m_start;
    CPos m_end;
    CPos m_leftUp;
    CPos m_rightDown;

};

class CValueNode : public CNode
{
private:
    CValue  m_val;

public:
    CValueNode(std::shared_ptr<std::map<CPos, CCell>> cells, CValue val) : CNode(cells), m_val(val) {}


    bool tryCyclicChain(std::set<CPos> visited) override
    {
        // todo
        return false;
    }

    CValue eval() override
    {
        return m_val;
    }
};

class CCell
{
public:
    CPos m_pos;
    mutable bool parsed = false;
    mutable CAst m_ast;
    mutable std::set<CPos> m_containing;
    CValue m_value;

    std::shared_ptr<std::map<CPos, CCell>> m_map;
    CCell(CPos pos, std::shared_ptr<std::map<CPos, CCell>> map);
    CValue getValue() const;
    bool setContents(CValue contents);
    CCell(const CCell &other);
    CCell &operator=(const CCell &other);
    bool checkCyclic() const;

    void buildTree() const;
};

class CCellNode : public CNode
{
private:
    CPos m_pos;
public:
    CCellNode(std::shared_ptr<std::map<CPos, CCell>> cells, CPos position) : CNode(cells), m_pos(position) {}

    CValue eval() override
    {
        auto it = m_cells->find(m_pos);
        // no idea how to fix this
        if (it != m_cells->end())
            return it->second.getValue();
        else
            return {};
    }

    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_cells->find(m_pos) == m_cells->end())
            return false;

        if(!m_cells->at(m_pos).parsed)
            m_cells->at(m_pos).buildTree();

        auto tmp = m_cells->at(m_pos).m_containing;
        for(auto &i : tmp)
        {
            if(visited.find(i) != visited.end())
                return true;

            visited.insert(i);
        }

        for(auto &pos : tmp)
        {
            if(m_cells->find(pos) == m_cells->end())
                return false;
            if(!m_cells->at(pos).parsed)
                m_cells->at(pos).buildTree();
            if(m_cells->at(pos).m_ast.m_root->tryCyclicChain(visited))
                return true;
        }

        return false;
    }
};

void fillVectorByRange(std::vector<std::shared_ptr<CNode>>& toFill, std::shared_ptr<std::map<CPos, CCell>> map, CRange range)
{
    toFill.push_back(std::make_shared<CCellNode>(map, range.m_start));
    while(true)
    {
        toFill.push_back(std::make_shared<CCellNode>(map, range.increment()));
        if(range.isEnd())
            break;
    }
}

class CSumNode : public CNode
{
    std::vector<std::shared_ptr<CNode>> m_cells;
    CRange m_range;

public:

    bool tryCyclicChain(std::set<CPos> visited) override
    {
        // todo
        return false;
    }

    CSumNode(std::shared_ptr<std::map<CPos, CCell>> map, CRange range) : CNode(map), m_range(range)
    {
        fillVectorByRange(m_cells, map, range);
    }

    CValue eval() override
    {
        double sum = 0;

        for(auto &cell : m_cells)
        {
            if(std::holds_alternative<double>(cell->eval()))
                sum += std::get<double>(cell->eval());
        }

        return sum;
    }

};

class CCountNode : public CNode
{
    std::vector<std::shared_ptr<CNode>> m_cells;
    CRange m_range;

public:

    CCountNode(std::shared_ptr<std::map<CPos, CCell>> map, CRange range) : CNode(map), m_range(range)
    {
        fillVectorByRange(m_cells, map, range);
    }

    bool tryCyclicChain(std::set<CPos> visited) override
    {
        // todo
        return false;
    }

    CValue eval() override
    {
        double count = 0;

        for(auto &cell : m_cells)
        {
            if(std::holds_alternative<double>(cell->eval()) || std::holds_alternative<std::string>(cell->eval()))
                count++;
        }

        return count;
    }
};

class CMinNode : public CNode
{
    std::vector<std::shared_ptr<CNode>> m_cells;
    CRange m_range;

public:
    CMinNode(std::shared_ptr<std::map<CPos, CCell>> map, CRange range) : CNode(map), m_range(range)
    {
        fillVectorByRange(m_cells, map, range);
    }

    bool tryCyclicChain(std::set<CPos> visited) override
    {
        // todo
        return false;
    }
    CValue eval() override
    {
        double min = std::numeric_limits<double>::max();

        for(auto &cell : m_cells)
        {
            if(std::holds_alternative<double>(cell->eval()))
                min = std::min(min, std::get<double>(cell->eval()));
        }

        return min;
    }
};

class CMaxNode : public CNode {
    std::vector<std::shared_ptr<CNode>> m_cells;
    CRange m_range;

public:
    CMaxNode(std::shared_ptr<std::map<CPos, CCell>> map, CRange range) : CNode(map), m_range(range)
    {
        fillVectorByRange(m_cells, map, range);
    }
    bool tryCyclicChain(std::set<CPos> visited) override
    {
        // todo
        return false;
    }


    CValue eval() override
    {
        double max = std::numeric_limits<double>::min();

        for(auto &cell : m_cells)
        {
            if(std::holds_alternative<double>(cell->eval()))
                max = std::max(max, std::get<double>(cell->eval()));
        }

        return max;
    }
};

class CCountValueNode : public CNode
{
    std::vector<std::shared_ptr<CNode>> m_cells;
    CRange m_range;
    CValue m_value;

public:
    CCountValueNode(std::shared_ptr<std::map<CPos, CCell>> map, CRange range, CValue value) : CNode(map), m_range(range), m_value(value)
    {
        fillVectorByRange(m_cells, map, range);
    }
    bool tryCyclicChain(std::set<CPos> visited) override
    {
        // todo
        return false;
    }

    CValue eval() override
    {
        double count = 0;

        for(auto &cell : m_cells)
        {
            if(cell->eval() == m_value)
                count++;
        }

        return count;
    }

};

class CIfNode : public CNode
{
    std::shared_ptr<CNode> m_condition;
    std::shared_ptr<CNode> m_true;
    std::shared_ptr<CNode> m_false;

public:
    CIfNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> condition, std::shared_ptr<CNode> trueNode, std::shared_ptr<CNode> falseNode) : CNode(map), m_condition(condition), m_true(trueNode), m_false(falseNode) {}
    bool tryCyclicChain(std::set<CPos> visited) override
    {
        // todo
        return false;
    }

    CValue eval() override
    {
        if(std::holds_alternative<double>(m_condition->eval()))
        {
            if(std::get<double>(m_condition->eval()) != 0)
                return m_true->eval();
            else
                return m_false->eval();
        }

        return {};
    }
};

class CAddNode : public CNode
{
    std::shared_ptr<CNode> m_left;
    std::shared_ptr<CNode> m_right;

public:
    CAddNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> left, std::shared_ptr<CNode> right) : CNode(map), m_left(left), m_right(right) {}

    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_left->tryCyclicChain(visited))
            return true;
        if(( m_right->tryCyclicChain(visited)))
            return true;

        return false;
    }

    CValue eval() override
    {
        if(std::holds_alternative<double>(m_left->eval()) && std::holds_alternative<double>(m_right->eval()))
            return std::get<double>(m_left->eval()) + std::get<double>(m_right->eval());
        else if(std::holds_alternative<std::string>(m_left->eval()) && std::holds_alternative<std::string>(m_right->eval()))
            return std::get<std::string>(m_left->eval()) + std::get<std::string>(m_right->eval());
        else if(std::holds_alternative<double>(m_left->eval()) && std::holds_alternative<std::string>(m_right->eval()))
            return std::to_string(std::get<double>(m_left->eval())) + std::get<std::string>(m_right->eval());
        else if(std::holds_alternative<std::string>(m_left->eval()) && std::holds_alternative<double>(m_right->eval()))
            return std::get<std::string>(m_left->eval()) + std::to_string(std::get<double>(m_right->eval()));
        else
            return {};
    }

};

class CSubNode : public CNode
{
    std::shared_ptr<CNode> m_left;
    std::shared_ptr<CNode> m_right;

public:

    CSubNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> left, std::shared_ptr<CNode> right) : CNode(map), m_left(left), m_right(right) {}

    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_left->tryCyclicChain(visited) || m_right->tryCyclicChain(visited))
            return true;
        return false;
    }

    CValue eval() override
    {
        if(std::holds_alternative<double>(m_left->eval()) && std::holds_alternative<double>(m_right->eval()))
            return std::get<double>(m_left->eval()) - std::get<double>(m_right->eval());
        else
            return {};
    }
};

class CMulNode : public CNode
{
    std::shared_ptr<CNode> m_left;
    std::shared_ptr<CNode> m_right;

public:
    CMulNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> left, std::shared_ptr<CNode> right) : CNode(map), m_left(left), m_right(right) {}

    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_left->tryCyclicChain(visited) || m_right->tryCyclicChain(visited))
            return true;
        return false;
    }
    CValue eval() override
    {
        if(std::holds_alternative<double>(m_left->eval()) && std::holds_alternative<double>(m_right->eval()))
            return std::get<double>(m_left->eval()) * std::get<double>(m_right->eval());
        else
            return {};
    }
};

class CDivNode : public CNode
{
    std::shared_ptr<CNode> m_left;
    std::shared_ptr<CNode> m_right;
public:
    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_left->tryCyclicChain(visited) || m_right->tryCyclicChain(visited))
            return true;
        return false;
    }

    CDivNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> left, std::shared_ptr<CNode> right) : CNode(map), m_left(left), m_right(right) {}

    CValue eval() override
    {
        if(std::get<double>(m_right->eval()) == 0)
            return {};

        if(std::holds_alternative<double>(m_left->eval()) && std::holds_alternative<double>(m_right->eval()))
            return std::get<double>(m_left->eval()) / std::get<double>(m_right->eval());
        else
            return {};
    }

};

class CPowNode : public CNode
{
    std::shared_ptr<CNode> m_left;
    std::shared_ptr<CNode> m_right;

public:
    CPowNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> left, std::shared_ptr<CNode> right) : CNode(map), m_left(left), m_right(right) {}

    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_left->tryCyclicChain(visited) || m_right->tryCyclicChain(visited))
            return true;
        return false;
    }
    CValue eval() override
    {
        if(std::holds_alternative<double>(m_left->eval()) && std::holds_alternative<double>(m_right->eval()))
            return pow(std::get<double>(m_left->eval()), std::get<double>(m_right->eval()));
        else
            return {};
    }

};

class CNegNode : public CNode
{
    std::shared_ptr<CNode> m_left;

public:
    CNegNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> left) : CNode(map), m_left(left) {}
    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_left->tryCyclicChain(visited))
            return true;
        return false;
    }

    CValue eval() override
    {
        if(std::holds_alternative<double>(m_left->eval()))
            return -std::get<double>(m_left->eval());
        else
            return {};
    }

};

class COpEqNode : public CNode
{
    std::shared_ptr<CNode> m_left;
    std::shared_ptr<CNode> m_right;
public:

    COpEqNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> left, std::shared_ptr<CNode> right) : CNode(map), m_left(left), m_right(right) {}
    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_left->tryCyclicChain(visited) || m_right->tryCyclicChain(visited))
            return true;
        return false;
    }

    CValue eval() override
    {
        if(std::holds_alternative<double>(m_left->eval()) && std::holds_alternative<double>(m_right->eval()))
            if(std::get<double>(m_left->eval()) == std::get<double>(m_right->eval()))
                return CValue {1.0};
            else
                return CValue {0.0};
        else if(std::holds_alternative<std::string>(m_left->eval()) && std::holds_alternative<std::string>(m_right->eval()))
            if(std::get<std::string>(m_left->eval()) == std::get<std::string>(m_right->eval()))
                return CValue {1.0};
            else
                return CValue {0.0};
        else
            return {};
    }

};

class CONotEqNode : public CNode
{
    std::shared_ptr<CNode> m_left;
    std::shared_ptr<CNode> m_right;

public:

    CONotEqNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> left, std::shared_ptr<CNode> right) : CNode(map), m_left(left), m_right(right) {}
    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_left->tryCyclicChain(visited) || m_right->tryCyclicChain(visited))
            return true;
        return false;
    }

    CValue eval() override
    {
        if(std::holds_alternative<double>(m_left->eval()) && std::holds_alternative<double>(m_right->eval()))
            if(std::get<double>(m_left->eval()) != std::get<double>(m_right->eval()))
                return CValue {1.0};
            else
                return CValue {0.0};
        else if(std::holds_alternative<std::string>(m_left->eval()) && std::holds_alternative<std::string>(m_right->eval()))
            if(std::get<std::string>(m_left->eval()) != std::get<std::string>(m_right->eval()))
                return CValue {1.0};
            else
                return CValue {0.0};
        else
            return {};
    }


};

class CLessThanNode : public CNode
{
    std::shared_ptr<CNode> m_left;
    std::shared_ptr<CNode> m_right;

public:
    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_left->tryCyclicChain(visited) || m_right->tryCyclicChain(visited))
            return true;
        return false;
    }
    CLessThanNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> left, std::shared_ptr<CNode> right) : CNode(map), m_left(left), m_right(right) {}

    CValue eval() override
    {
        if(std::holds_alternative<double>(m_left->eval()) && std::holds_alternative<double>(m_right->eval()))
            if(std::get<double>(m_left->eval()) < std::get<double>(m_right->eval()))
                return CValue {1.0};
            else
                return CValue {0.0};
        else if(std::holds_alternative<std::string>(m_left->eval()) && std::holds_alternative<std::string>(m_right->eval()))
            if(std::get<std::string>(m_left->eval()) < std::get<std::string>(m_right->eval()))
                return CValue {1.0};
            else
                return CValue {0.0};
        else
            return {};
    }
};

class CLessEqualNode : public CNode
{
    std::shared_ptr<CNode> m_left;
    std::shared_ptr<CNode> m_right;

public:
    CLessEqualNode(std::shared_ptr<std::map<CPos, CCell>> map, std::shared_ptr<CNode> left, std::shared_ptr<CNode> right) : CNode(map), m_left(left), m_right(right) {}
    bool tryCyclicChain(std::set<CPos> visited) override
    {
        if(m_left->tryCyclicChain(visited) || m_right->tryCyclicChain(visited))
            return true;
        return false;
    }

    CValue eval() override
    {
        if(std::holds_alternative<double>(m_left->eval()) && std::holds_alternative<double>(m_right->eval()))
            if(std::get<double>(m_left->eval()) <= std::get<double>(m_right->eval()))
                return CValue {1.0};
            else
                return CValue {0.0};
        else if(std::holds_alternative<std::string>(m_left->eval()) && std::holds_alternative<std::string>(m_right->eval()))
            if(std::get<std::string>(m_left->eval()) <= std::get<std::string>(m_right->eval()))
                return CValue {1.0};
            else
                return CValue {0.0};
        else
            return {};
    }

};

class CMyExprBuilder : public CExprBuilder
{
public:
    std::set<CPos> m_all_cells;
    CAst m_ast;
    std::stack<std::shared_ptr<CNode>> m_stack;
    std::shared_ptr<std::map<CPos, CCell>> m_map;

    CMyExprBuilder(std::shared_ptr<std::map<CPos, CCell>> map) : m_map(map) {}

    std::pair<std::shared_ptr<CNode>, std::shared_ptr<CNode>> popTwo()
    {
        if (m_stack.size() < 2)
            throw std::invalid_argument("Invalid expression");

        std::shared_ptr<CNode> right = m_stack.top();
        m_stack.pop();
        std::shared_ptr<CNode> left = m_stack.top();
        m_stack.pop();

        return {left, right};
    }

    void opAdd() override {
        auto both = popTwo();

        std::shared_ptr<CAddNode> addNode = std::make_shared<CAddNode>(m_map, both.first, both.second);
        m_stack.push(addNode);

        // remove last two nodes from m_ast
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(addNode);
        m_ast.m_root = addNode;
    }

    void opSub() override
    {
        auto both = popTwo();

        std::shared_ptr<CSubNode> subNode = std::make_shared<CSubNode>(m_map, both.first, both.second);
        m_stack.push(subNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(subNode);
        m_ast.m_root = subNode;
    }

    void opMul() override
    {
        auto both = popTwo();

        std::shared_ptr<CMulNode> mulNode = std::make_shared<CMulNode>(m_map, both.first, both.second);
        m_stack.push(mulNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(mulNode);
        m_ast.m_root = mulNode;
    }

    void opDiv() override
    {
        auto both = popTwo();

        std::shared_ptr<CDivNode> divNode = std::make_shared<CDivNode>(m_map, both.first, both.second);
        m_stack.push(divNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(divNode);
        m_ast.m_root = divNode;
    }

    void opPow() override
    {
        auto both = popTwo();

        std::shared_ptr<CPowNode> powNode = std::make_shared<CPowNode>(m_map, both.first, both.second);
        m_stack.push(powNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(powNode);
        m_ast.m_root = powNode;
    }

    void opNeg() override
    {
        if (m_stack.empty())
            throw std::invalid_argument("Invalid expression");

        std::shared_ptr<CNode> left = m_stack.top();
        m_stack.pop();

        std::shared_ptr<CNegNode> negNode = std::make_shared<CNegNode>(m_map, left);
        m_stack.push(negNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(negNode);

        m_ast.m_root = negNode;
    }

    void opEq() override
    {
        auto both = popTwo();

        std::shared_ptr<COpEqNode> eqNode = std::make_shared<COpEqNode>(m_map, both.first, both.second);
        m_stack.push(eqNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(eqNode);
        m_ast.m_root = eqNode;
    }

    void opNe() override
    {
        auto both = popTwo();
        std::shared_ptr<CONotEqNode> neNode = std::make_shared<CONotEqNode>(m_map, both.first, both.second);
        m_stack.push(neNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(neNode);
        m_ast.m_root = neNode;
    }

    void opLt() override
    {
        auto both = popTwo();

        std::shared_ptr<CLessThanNode> ltNode = std::make_shared<CLessThanNode>(m_map, both.first, both.second);
        m_stack.push(ltNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(ltNode);
        m_ast.m_root = ltNode;
    }

    void opLe() override
    {
        auto both = popTwo();
        std::shared_ptr<CLessEqualNode> leNode = std::make_shared<CLessEqualNode>(m_map, both.first, both.second);
        m_stack.push(leNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(leNode);
        m_ast.m_root = leNode;
    }

    void opGt() override
    {
        auto both = popTwo();

        std::shared_ptr<CLessThanNode> ltNode = std::make_shared<CLessThanNode>(m_map, both.second, both.first);
        m_stack.push(ltNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(ltNode);
        m_ast.m_root = ltNode;
    }

    void opGe() override
    {
        auto both = popTwo();

        std::shared_ptr<CLessEqualNode> leNode = std::make_shared<CLessEqualNode>(m_map, both.second, both.first);
        m_stack.push(leNode);

        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.pop_back();
        m_ast.m_currentLayer.push_back(leNode);
        m_ast.m_root = leNode;
    }

    void valNumber(double val) override
    {
        std::shared_ptr<CValueNode> node = std::make_shared<CValueNode>(m_map, val);
        m_stack.push(node);

        m_ast.m_currentLayer.push_back(node);
        if(m_ast.m_root == nullptr)
            m_ast.m_root = node;
    }

    void valString(std::string val) override
    {
        std::shared_ptr<CValueNode> node = std::make_shared<CValueNode>(m_map, val);
        m_stack.push(node);
        m_ast.m_currentLayer.push_back(node);

        if(m_ast.m_root == nullptr)
            m_ast.m_root = node;
    }

    void valReference(std::string val) override
    {
        std::shared_ptr<CCellNode> node = std::make_shared<CCellNode>(m_map, CPos(val));
        m_all_cells.insert(CPos(val));
        m_stack.push(node);
        m_ast.m_currentLayer.push_back(node);

        if(m_ast.m_root == nullptr)
            m_ast.m_root = node;
    }

    void valRange(std::string val) override
    {
        auto left = val.substr(0, val.find(':'));
        auto right = val.substr(val.find(':') + 1);

        std::shared_ptr<CRange> range = std::make_shared<CRange>(CPos(left), CPos(right));
        m_stack.push(range);
        m_ast.m_currentLayer.push_back(range);
        if(m_ast.m_root == nullptr)
            m_ast.m_root = range;
    }

    void funcCall(std::string fnName, int paramCount) override
    {
        std::shared_ptr<CNode> node;
        // todo check if i need cast -- not working anyway
        if(fnName == "sum")
        {
            auto range = std::dynamic_pointer_cast<CRange>(m_stack.top());
            m_stack.pop();
            node = std::make_shared<CSumNode>(m_map, *range);
            m_stack.push(node);
        }
        else if(fnName == "count")
        {
            auto range = std::dynamic_pointer_cast<CRange>(m_stack.top());
            m_stack.pop();
            node = std::make_shared<CCountNode>(m_map, *range);
            m_stack.push(node);
        }
        else if(fnName == "min")
        {
            auto range = std::dynamic_pointer_cast<CRange>(m_stack.top());
            m_stack.pop();
            node = std::make_shared<CMinNode>(m_map, *range);
            m_stack.push(node);
        }
        else if(fnName == "max")
        {
            auto range = std::dynamic_pointer_cast<CRange>(m_stack.top());
            m_stack.pop();
            node = std::make_shared<CMaxNode>(m_map, *range);
            m_stack.push(node);
        }
        else if(fnName == "countValue")
        {
            auto range = std::dynamic_pointer_cast<CRange>(m_stack.top());
            m_stack.pop();
            auto value = m_stack.top();
            m_stack.pop();
            node = std::make_shared<CCountValueNode>(m_map, *range, value->eval());
            m_stack.push(node);
        }
        else if(fnName == "if")
        {
            auto falseNode = m_stack.top();
            m_stack.pop();
            auto trueNode = m_stack.top();
            m_stack.pop();
            auto condition = m_stack.top();
            m_stack.pop();
            node = std::make_shared<CIfNode>(m_map, condition, trueNode, falseNode);
            m_stack.push(node);
        }
        else
            throw std::invalid_argument("Invalid function");

        m_ast.m_currentLayer.push_back(node);
        if(m_ast.m_root == nullptr)
            m_ast.m_root = node;
    }
};

bool CCell::checkCyclic() const
{
    std::set<CPos> visited = {m_pos};
    return m_ast.m_root->tryCyclicChain(visited);
}

CCell::CCell(CPos pos, std::shared_ptr<std::map<CPos, CCell>> map) : m_pos(pos), m_map(map) {}

CCell::CCell(const CCell &other) : m_pos(other.m_pos), m_map(other.m_map)
{
    m_value = other.m_value;
    m_ast.m_root = nullptr;
    parsed = false;
}

CCell &CCell::operator=(const CCell &other)
{
    if(this == &other)
        return *this;

    m_pos = other.m_pos;

    // WARNING - WRONG POINTER
    m_map = other.m_map;
    m_value = other.m_value;
    m_ast.m_root = nullptr;
    parsed = false;
    return *this;
}

CValue CCell::getValue() const
{
    if(!parsed) {
        buildTree();
    }
    if(checkCyclic())
       return {}; 
    
    return m_ast.m_root->eval();
}


bool CCell::setContents(CValue contents) // check validity
{
    parsed = false;
    auto restore = m_value;
    m_value = contents;
    try
    {
        buildTree();
    }
    catch (...)
    {
        m_value = restore;
        parsed = false;
        return false;
    }

    m_value = contents;
    return true;
}

void CCell::buildTree() const
{
    m_containing.clear();
    CMyExprBuilder builder(m_map);
    parseExpression(std::get<std::string>(m_value), builder);
    m_ast.m_root = builder.m_ast.m_root;
    m_containing = builder.m_all_cells;
    parsed = true;
}

void replacePos(std::string& original, std::string toReplace, int width, int height, uint& start, uint& end)
{
    CPos pos(toReplace);
    if(!pos.m_lockCol1) {
        if (width >= 0)
            for (int i = 0; i < width; i++)
                pos.incrementCol();
        else
            for (int i = 0; i > width; i--)
                pos.decrementCol();
    }

    if(!pos.m_lockRow1 && pos.m_row1 + height < 0)
        throw std::invalid_argument("Invalid replacement");
    else if(!pos.m_lockRow1)
        pos.m_row1 += height;
    toReplace = pos.toString();

    original.replace(start, end - start, toReplace);
    end = start + toReplace.size();
    start = end;

}

// used in copyRect - easiest way to increment all relative values and keep m_value and have parsed=false in new cell
// m_value has to be valid - used in copy constructor, = operator, save and load
std::string incrementAllRelative(std::string original, int width, int height)
{
    original.erase(remove(original.begin(), original.end(), ' '), original.end());
    uint i = 0;

    if(original[i] == '=')
        i++;
    else
        return original;


    uint state = 0;
    uint startOfCurrentCell = 0;
    std::string currentCell;

    while(true) {
        switch (state) {
            // no cell previously opened
            case 0:
                END_FINITE_STATE_MACHINE

                currentCell = "";
                if (isalpha(original[i])) {
                    state = 1;
                    startOfCurrentCell = i;
                } else if (original[i] == '$') {
                    startOfCurrentCell = i;
                    state = 1;
                    currentCell += original[i];
                    i++;
                    END_FINITE_STATE_MACHINE
                } else {
                    state = 0;
                    i++;
                }
                break;

                // reading column
            case 1:
                // there is only a $
                if (!isalpha(original[i])) {
                    state = 0;
                    i++;
                    break;
                }

                currentCell += original[i];
                i++;
                END_FINITE_STATE_MACHINE

                if (isdigit(original[i]))
                    state = 2;
                else if (original[i] == '$') {
                    state = 2;
                    currentCell += original[i];
                    i++;
                    END_FINITE_STATE_MACHINE

                } else if (isalpha(original[i]))
                    state = 1;
                else
                    state = 0;
                break;

                // reading row
            case 2:
                // there is only a $
                if (!isdigit(original[i])) {
                    state = 0;
                    i++;
                    break;
                }

                currentCell += original[i];
                i++;
                if (i >= original.size()) {
                    state = 3;
                    break;
                }

                if (isdigit(original[i]))
                    state = 2;
                else
                    state = 3;

                break;

            case 3:
                replacePos(original, currentCell, width, height, startOfCurrentCell, i);

                END_FINITE_STATE_MACHINE
                state = 0;
                break;

            case 4:
                return original;

            default:
                state = 0;
                break;
        }
    }
    return original;
}

class CSpreadsheet {
public:
    static unsigned capabilities() {
        return SPREADSHEET_CYCLIC_DEPS;
    }

    CSpreadsheet() : m_cells(std::make_shared<std::map<CPos, CCell>>()) {}

    CSpreadsheet(const CSpreadsheet &other) {
        m_cells = std::make_shared<std::map<CPos, CCell>>();
        for (auto &cell: *other.m_cells) {
            CCell newCCell = cell.second;
            newCCell.m_map = m_cells;
            m_cells->insert({cell.first, newCCell});
        }
    }

    CSpreadsheet &operator=(const CSpreadsheet &other) {
        if (this == &other)
            return *this;

        m_cells = std::make_shared<std::map<CPos, CCell>>();
        for (auto &cell: *other.m_cells) {
            CCell newCCell = cell.second;
            newCCell.m_map = m_cells;
            m_cells->insert({cell.first, newCCell});
        }

        return *this;
    }

    ~CSpreadsheet() {
        m_cells->clear();
    }

    bool setCell(CPos pos, std::string contents) {
        CCell cell(pos, m_cells);

        if(contents.empty())
        {
            m_cells->erase(pos);
            return true;
        }

        // if container already contains cell with this position, update it
        m_cells->insert({pos, cell});
        return m_cells->find(pos)->second.setContents(contents);

    }

    CValue getValue(CPos pos) {
        if (m_cells->find(pos) == m_cells->end())
            return {};

        return m_cells->find(pos)->second.getValue();
    }

    CValue getStringValue(CPos pos) {
        if (m_cells->find(pos) == m_cells->end())
            return "";

        return m_cells->find(pos)->second.m_value;
    }

    bool save(std::ostream &os) const
    {
        for(auto &cell : *m_cells)
        {
            if(!os)
                return false;

            if(std::holds_alternative<double>(cell.second.m_value))
                os << cell.first.toString() << SEPARATORPOSVAL << std::get<double>(cell.second.m_value) << SEPARATORVALPOS;
            else
                os << cell.first.toString() << SEPARATORPOSVAL << std::get<std::string>(cell.second.m_value) << SEPARATORVALPOS;
        }
        return true;
    }


    bool load(std::istream &is)
    {
        m_cells->clear();
        if(!is)
            return false;

        std::istreambuf_iterator<char> eos;
        std::string content(std::istreambuf_iterator<char>(is), eos);

        while(true)
        {
            if(content.empty())
                break;

            std::string pos = content.substr(0, content.find(SEPARATORPOSVAL));
            content = content.substr(content.find(SEPARATORPOSVAL) + strlen((SEPARATORPOSVAL)));

            std::string value = content.substr(0, content.find(SEPARATORVALPOS));
            content = content.substr(content.find(SEPARATORVALPOS) + strlen(SEPARATORVALPOS));

            try {
                CCell cell(CPos(pos), m_cells);
                m_cells->insert({CPos(pos), cell});
                setCell(CPos(pos), value);
            }
            catch (...)
            { return false; }
        }

        return true;
    }

    void copyRect(CPos dst, CPos src, int w = 1, int h = 1)
    {
        std::vector<CValue> tmpCopy;
        CPos right_src = src;
        right_src.m_row1 += (h - 1);

        CPos right_dst = dst;
        right_dst.m_row1 += (h - 1);

        for(int i = 0; i < (w - 1); i++)
        {
            right_src.incrementCol();
            right_dst.incrementCol();
        }

        CRange src_range(src, right_src);
        CRange dst_range(dst, right_dst);

        tmpCopy.push_back(getStringValue(src));
        while(true)
        {
            tmpCopy.push_back(getStringValue(src_range.increment()));
            if(src_range.isEnd())
                break;
        }

        if(!std::get<std::string>(tmpCopy[0]).empty())
            setCell(dst, incrementAllRelative(std::get<std::string>(tmpCopy[0]), CPos::difference(dst.m_col1, src.m_col1), dst.m_row1 - src.m_row1));
        else
            (*m_cells).erase(dst);

        uint i = 1;
        while(i < tmpCopy.size())
        {
            CPos tmp = dst_range.increment();

            if(!std::get<std::string>(tmpCopy[i]).empty())
                setCell(tmp, incrementAllRelative(std::get<std::string>(tmpCopy[i]), CPos::difference(dst.m_col1, src.m_col1), dst.m_row1 - src.m_row1));
            else
                (*m_cells).erase(tmp);

            i++;
            if(dst_range.isEnd())
                break;
        }

    }

    std::shared_ptr<std::map<CPos, CCell>> m_cells;
};

#ifndef __PROGTEST__

bool valueMatch(const CValue &r,
                const CValue &s) {
    if (r.index() != s.index())
        return false;
    if (r.index() == 0)
        return true;
    if (r.index() == 2)
        return std::get<std::string>(r) == std::get<std::string>(s);
    if (std::isnan(std::get<double>(r)) && std::isnan(std::get<double>(s)))
        return true;
    if (std::isinf(std::get<double>(r)) && std::isinf(std::get<double>(s)))
        return (std::get<double>(r) < 0 && std::get<double>(s) < 0)
               || (std::get<double>(r) > 0 && std::get<double>(s) > 0);
    return fabs(std::get<double>(r) - std::get<double>(s)) <= 1e8 * DBL_EPSILON * fabs(std::get<double>(r));
}



void testReplacePos()
{
    assert(incrementAllRelative("=A1", 1, 1) == "=B2");
    assert(incrementAllRelative("=A1", 1, -1) == "=B0");
    assert(incrementAllRelative("=B1", -1, 1) == "=A2");
    assert(incrementAllRelative("=B1", -1, -1) == "=A0");
    assert(incrementAllRelative("=B1", 0, 0) == "=B1");
    assert(incrementAllRelative("=B1", 0, 1) == "=B2");
    assert(incrementAllRelative("=B1", 1, 0) == "=C1");
    assert(incrementAllRelative("=B1", 0, -1) == "=B0");
    assert(incrementAllRelative("=B1", -1, 0) == "=A1");




}

void copyLoadSaveTest()
{
    CSpreadsheet x0, x1;
    std::ostringstream oss;
    std::istringstream iss;
    std::string data;

    // how to load and save
    oss . clear ();
    oss . str ( "" );
    assert ( x0 . save ( oss ) );
    data = oss . str ();
    iss . clear ();
    iss . str ( data );
    assert ( x1 . load ( iss ) );
    // --------------------------------------------------------

}

void anotherCyclicTest()
{
    
}

void cyclicTest()
{
    anotherCyclicTest();

    CSpreadsheet x1;
    assert ( x1 . setCell ( CPos ( "A1" ), "=A2" ) );
    assert ( x1 . setCell ( CPos ( "A2" ), "=A1" ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "A1" ) ), CValue() ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "A2" ) ), CValue() ) );

    assert ( x1 . setCell ( CPos ( "A1" ), "5" ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "A1" ) ), CValue ( 5.0 ) ) );
    assert ( x1 . setCell ( CPos ( "A2" ), "=A1+5" ) );
    assert ( x1 . setCell ( CPos ( "A3" ), "=A1+A2+5" ) );

    assert ( valueMatch ( x1 . getValue ( CPos ( "A1" ) ), CValue ( 5.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "A2" ) ), CValue ( 10.0 ) ) );
    assert(valueMatch ( x1.getValue(CPos("A3")), CValue(20.0) ) );

    assert(x1.setCell(CPos("A4"), "=A3"));
    assert(x1.setCell(CPos("A1"), "=A4"));
    assert(x1.getValue(CPos("A1")) == CValue());

    assert(x1.setCell(CPos("A1"), ""));
    assert(x1.getValue(CPos("A1")) == CValue());
    assert(x1.getValue(CPos("A2")) == CValue());
    assert(x1.getValue(CPos("A3")) == CValue());
    assert(x1.getValue(CPos("A4")) == CValue());

    auto x3 = x1;
    assert(x3.setCell(CPos("A1"), "5"));
    assert(valueMatch ( x3.getValue(CPos("A3")), CValue(20.0) ) );

    CSpreadsheet x0;
    assert ( x0 . setCell ( CPos ( "A1" ), "=A2" ) );
    assert ( x0 . setCell ( CPos ( "A3" ), "5" ) );
    assert ( x0 . setCell ( CPos ( "A2" ), "=5+A3+A1" ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "A1" ) ), CValue() ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "A2" ) ), CValue() ) );
}

int main()
{
    cyclicTest();

    CSpreadsheet x0, x1;
    std::ostringstream oss;
    std::istringstream iss;
    std::string data;
    assert ( x0 . setCell ( CPos ( "A1" ), "10" ) );
    assert ( x0 . setCell ( CPos ( "A2" ), "20.5" ) );
    assert ( x0 . setCell ( CPos ( "A3" ), "3e1" ) );
    assert ( x0 . setCell ( CPos ( "A4" ), "=40" ) );
    assert ( x0 . setCell ( CPos ( "A5" ), "=5e+1" ) );
    assert ( x0 . setCell ( CPos ( "A6" ), "raw text with any characters, including a quote \" or a newline\n" ) );
    assert ( x0 . setCell ( CPos ( "A7" ), "=\"quoted string, quotes must be doubled: \"\". Moreover, backslashes are needed for C++.\"" ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "A1" ) ), CValue ( 10.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "A2" ) ), CValue ( 20.5 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "A3" ) ), CValue ( 30.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "A4" ) ), CValue ( 40.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "A5" ) ), CValue ( 50.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "A6" ) ), CValue ( "raw text with any characters, including a quote \" or a newline\n" ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "A7" ) ), CValue ( "quoted string, quotes must be doubled: \". Moreover, backslashes are needed for C++." ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "A8" ) ), CValue() ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "AAAA9999" ) ), CValue() ) );
    assert ( x0 . setCell ( CPos ( "B1" ), "=A1+A2*A3" ) );
    assert ( x0 . setCell ( CPos ( "B2" ), "= -A1 ^ 2 - A2 / 2   " ) );
    assert ( x0 . setCell ( CPos ( "B3" ), "= 2 ^ $A$1" ) );
    assert ( x0 . setCell ( CPos ( "B4" ), "=($A1+A$2)^2" ) );
    assert ( x0 . setCell ( CPos ( "B5" ), "=B1+B2+B3+B4" ) );
    assert ( x0 . setCell ( CPos ( "B6" ), "=B1+B2+B3+B4+B5" ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B1" ) ), CValue ( 625.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B2" ) ), CValue ( -110.25 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B3" ) ), CValue ( 1024.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B4" ) ), CValue ( 930.25 ) ) );

    assert ( valueMatch ( x0 . getValue ( CPos ( "B5" ) ), CValue ( 2469.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B6" ) ), CValue ( 4938.0 ) ) );
    assert ( x0 . setCell ( CPos ( "A1" ), "12" ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B1" ) ), CValue ( 627.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B2" ) ), CValue ( -154.25 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B3" ) ), CValue ( 4096.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B4" ) ), CValue ( 1056.25 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B5" ) ), CValue ( 5625.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B6" ) ), CValue ( 11250.0 ) ) );
    x1 = x0;
    assert ( x0 . setCell ( CPos ( "A2" ), "100" ) );
    assert ( x1 . setCell ( CPos ( "A2" ), "=A3+A5+A4" ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B1" ) ), CValue ( 3012.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B2" ) ), CValue ( -194.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B3" ) ), CValue ( 4096.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B4" ) ), CValue ( 12544.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B5" ) ), CValue ( 19458.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "B6" ) ), CValue ( 38916.0 ) ) );

    assert ( valueMatch ( x1 . getValue ( CPos ( "B1" ) ), CValue ( 3612.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B2" ) ), CValue ( -204.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B3" ) ), CValue ( 4096.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B4" ) ), CValue ( 17424.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B5" ) ), CValue ( 24928.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B6" ) ), CValue ( 49856.0 ) ) );
    oss . clear ();
    oss . str ( "" );
    assert ( x0 . save ( oss ) );
    data = oss . str ();
    iss . clear ();
    iss . str ( data );
    assert ( x1 . load ( iss ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B1" ) ), CValue ( 3012.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B2" ) ), CValue ( -194.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B3" ) ), CValue ( 4096.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B4" ) ), CValue ( 12544.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B5" ) ), CValue ( 19458.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B6" ) ), CValue ( 38916.0 ) ) );
    assert ( x0 . setCell ( CPos ( "A3" ), "4e1" ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B1" ) ), CValue ( 3012.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B2" ) ), CValue ( -194.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B3" ) ), CValue ( 4096.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B4" ) ), CValue ( 12544.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B5" ) ), CValue ( 19458.0 ) ) );
    assert ( valueMatch ( x1 . getValue ( CPos ( "B6" ) ), CValue ( 38916.0 ) ) );
    oss . clear ();
    oss . str ( "" );
    assert ( x0 . save ( oss ) );
    data = oss . str ();
    for ( size_t i = 0; i < std::min<size_t> ( data . length (), 10 ); i ++ )
        data[i] ^=0x5a;
    iss . clear ();
    iss . str ( data );
    assert ( ! x1 . load ( iss ) );
    assert ( x0 . setCell ( CPos ( "D0" ), "10" ) );
    assert ( x0 . setCell ( CPos ( "D1" ), "20" ) );
    assert ( x0 . setCell ( CPos ( "D2" ), "30" ) );
    assert ( x0 . setCell ( CPos ( "D3" ), "40" ) );
    assert ( x0 . setCell ( CPos ( "D4" ), "50" ) );
    assert ( x0 . setCell ( CPos ( "E0" ), "60" ) );
    assert ( x0 . setCell ( CPos ( "E1" ), "70" ) );
    assert ( x0 . setCell ( CPos ( "E2" ), "80" ) );
    assert ( x0 . setCell ( CPos ( "E3" ), "90" ) );
    assert ( x0 . setCell ( CPos ( "E4" ), "100" ) );
    assert ( x0 . setCell ( CPos ( "F10" ), "=D0+5" ) );
    assert ( x0 . setCell ( CPos ( "F11" ), "=$D0+5" ) );
    assert ( x0 . setCell ( CPos ( "F12" ), "=D$0+5" ) );
    assert ( x0 . setCell ( CPos ( "F13" ), "=$D$0+5" ) );
    x0 . copyRect ( CPos ( "G11" ), CPos ( "F10" ), 1, 4 );
    assert ( valueMatch ( x0 . getValue ( CPos ( "F10" ) ), CValue ( 15.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "F11" ) ), CValue ( 15.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "F12" ) ), CValue ( 15.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "F13" ) ), CValue ( 15.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "F14" ) ), CValue() ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "G10" ) ), CValue() ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "G11" ) ), CValue ( 75.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "G12" ) ), CValue ( 25.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "G13" ) ), CValue ( 65.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "G14" ) ), CValue ( 15.0 ) ) );
    x0 . copyRect ( CPos ( "G11" ), CPos ( "F10" ), 2, 4 );
    assert ( valueMatch ( x0 . getValue ( CPos ( "F10" ) ), CValue ( 15.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "F11" ) ), CValue ( 15.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "F12" ) ), CValue ( 15.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "F13" ) ), CValue ( 15.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "F14" ) ), CValue() ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "G10" ) ), CValue() ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "G11" ) ), CValue ( 75.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "G12" ) ), CValue ( 25.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "G13" ) ), CValue ( 65.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "G14" ) ), CValue ( 15.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "H10" ) ), CValue() ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "H11" ) ), CValue() ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "H12" ) ), CValue() ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "H13" ) ), CValue ( 35.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "H14" ) ), CValue() ) );
    assert ( x0 . setCell ( CPos ( "F0" ), "-27" ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "H14" ) ), CValue ( -22.0 ) ) );

    //dst, src, width, height
    x0 . copyRect ( CPos ( "H12" ), CPos ( "H13" ), 1, 2 );
//    cout << get<double>(x0 . getValue ( CPos ( "H12" ) )) << endl;
    assert ( valueMatch ( x0 . getValue ( CPos ( "H12" ) ), CValue ( 25.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "H13" ) ), CValue ( -22.0 ) ) );
    assert ( valueMatch ( x0 . getValue ( CPos ( "H14" ) ), CValue ( -22.0 ) ) );

    testReplacePos();
    copyLoadSaveTest();

    return EXIT_SUCCESS;
}

// TODO - porovnavani stringu mi asi utrhne prdel
// TODO - je dobry pouzivat holds_alternative
// TODO - je dobry pouzivat dynamic_pointer_cast
// TODO - test load stringu, ktery obsahuje ; a test =bullshit
// TODO - zkontrolovat zo ma byt nedefinovane a co mam nedefinovane - CValue()
// TODO test increment a decrement row
/* IMPLEMENTACNI POZNAMKY
 */

#endif /* __PROGTEST__ */

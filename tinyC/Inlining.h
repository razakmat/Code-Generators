#pragma once

#include "IR.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace tinyc {

    class Inlining{
        public:
            Inlining(std::vector<Function*> & funs);
            void Start();
        protected:
            bool SearchFun(Function * fun);
            bool Inlineable(Function * fun);
            int GetSize(Function * fun);
            bool HasCall(Function * fun);
            void InlineFun(Function * fun,int block_index, int call_index, Function * callee);
            void PreparingInline(Function * caller, Function * callee, int block_index, int & call_index);
            void ChangeToNewVars(Function * fun, Function * callee);
            void StoreParamToArgs(Function * caller,Block * block, int & call_index);
            void EraseCallBorders(Block * block,int & call_index);
            void CreateBlocks(Function * caller,Function * callee, int block_index);
            void ChangeBlockWithCall(Function * caller,Block * block, int call_index,int block_index);
            void FillBlock(Block * to, Block * from);
            std::vector<Function*> & m_funs;
            std::unordered_set<Function*> m_fun_available;
            std::unordered_map<Instruction*,Instruction*> m_map_allocs;
            std::unordered_map<Instruction*,Instruction*> m_map_ins;
            std::unordered_map<Block*,Block*> m_map_block;
            Instruction * m_call_address;
            Block * m_return_block;
            int m_size_limit = 30;
    };
}
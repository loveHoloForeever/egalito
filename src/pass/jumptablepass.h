#ifndef EGALITO_PASS_JUMP_TABLE_PASS_H
#define EGALITO_PASS_JUMP_TABLE_PASS_H

#include <map>
#include "chunkpass.h"

/** Constructs jump table data structures in the given Module. */
class JumpTablePass : public ChunkPass {
private:
    Module *module;
    std::map<address_t, JumpTable *> tableMap;
public:
    JumpTablePass(Module *module = nullptr) : module(module) {}
    virtual void visit(Module *module);
    virtual void visit(JumpTableList *jumpTableList);
};

#endif
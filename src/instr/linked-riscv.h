#ifndef EGALITO_INSTR_LINKED_RISCV_H
#define EGALITO_INSTR_LINKED_RISCV_H

#include <vector>
#include "semantic.h"
// #include "concrete.h"
#include "chunk/chunkfwd.h"

#ifdef ARCH_RISCV

class LinkedInstruction : public LinkDecorator<SemanticImpl> {
private:
    Instruction *instruction;
    int opIndex;
    size_t displacementSize;
    size_t displacementOffset;
public:
    LinkedInstruction(Instruction *i) : instruction(i), opIndex(-1),
        displacementSize(0), displacementOffset(0) {}

    virtual void accept(InstructionVisitor *visitor) { visitor->visit(this); }

    Instruction *getSource() const { return instruction; }
    std::string getMnemonic() { return getAssembly()->getMnemonic(); }

    void setInstruction(Instruction *instruction)
        { this->instruction = instruction; }

    void writeTo(char *target, bool useDisp);
    void writeTo(std::string &target, bool useDisp);
    uint32_t rebuild();
};

// XXX: may not be used for RISCV?
class LinkedLiteralInstruction : public SemanticImpl {
public:
};

class ControlFlowInstruction : public LinkedInstruction {
private:
    bool nonreturn;
public:
    ControlFlowInstruction(Instruction *instruction)
        : LinkedInstruction(instruction), nonreturn(false) {}

    virtual void accept(InstructionVisitor *visitor) { visitor->visit(this); }

    bool returns() const { return !nonreturn; }
    void setNonreturn() { nonreturn = true; }
};

#endif

#endif
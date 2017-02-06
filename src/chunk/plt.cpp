#include "plt.h"
#include "elf/symbol.h"
#include "log/log.h"

std::string PLTEntry::getName() const {
    return getTargetSymbol()->getName() + std::string("@plt");
}

void PLTSection::parse(ElfMap *elf) {
    auto header = static_cast<Elf64_Shdr *>(elf->findSectionHeader(".plt"));
    auto section = reinterpret_cast<address_t>(elf->findSection(".plt"));

    PLTRegistry *registry = new PLTRegistry();
    for(auto r : *relocList) {
        if(r->getType() == R_X86_64_JUMP_SLOT) {
            LOG(1, "PLT entry at " << r->getAddress());
            registry->add(r->getAddress(), r);
        }
    }

#ifdef ARCH_X86_64
    static const size_t ENTRY_SIZE = 16;

    /* example format
        0000000000000550 <.plt>:
         550:   ff 35 b2 0a 20 00       pushq  0x200ab2(%rip)
         556:   ff 25 b4 0a 20 00       jmpq   *0x200ab4(%rip)
         55c:   0f 1f 40 00             nopl   0x0(%rax)

        0000000000000560 <puts@plt>:
         560:   ff 25 b2 0a 20 00       jmpq   *0x200ab2(%rip)
         566:   68 00 00 00 00          pushq  $0x0
         56b:   e9 e0 ff ff ff          jmpq   550 <.plt>
    */

    // note: we skip the first PLT entry, which has a different format
    for(size_t i = 1 * ENTRY_SIZE; i < header->sh_size; i += ENTRY_SIZE) {
        auto entry = section + i;

        LOG(1, "CONSIDER PLT entry at " << entry);

        if(*reinterpret_cast<const unsigned short *>(entry) == 0x25ff) {
            address_t pltAddress = header->sh_addr + i;
            address_t value = *reinterpret_cast<const unsigned int *>(entry + 2)
                + (pltAddress + 2+4);  // target is RIP-relative
            LOG(1, "PLT value would be " << value);
            Reloc *r = registry->find(value);
            if(r) {
                LOG(1, "Found PLT entry at " << pltAddress << " -> ["
                    << r->getSymbol()->getName() << "]");
                entryMap[pltAddress] = new PLTEntry(
                    pltAddress, r->getSymbol());
            }
        }
    }
#else
    static const size_t ENTRY_SIZE = 16;

    /* example format
        0000000000400420 <puts@plt>:
        400420:       90000090        adrp    x16, 410000 <__FRAME_END__+0xf9c8>
        400424:       f9443611        ldr     x17, [x16,#2152]
        400428:       9121a210        add     x16, x16, #0x868
        40042c:       d61f0220        br      x17
    */

    // note: we skip the first PLT entry, which is 2x the size of others
    for(size_t i = 2 * ENTRY_SIZE; i < header->sh_size; i += ENTRY_SIZE) {
        auto entry = section + i;

        LOG(1, "CONSIDER PLT entry at " << entry);
        LOG(1, "1st instr is " << (int)*reinterpret_cast<const unsigned int *>(entry));
        LOG(1, "2nd instr is " << (int)*reinterpret_cast<const unsigned int *>(entry+4*1));
        LOG(1, "3nd instr is " << (int)*reinterpret_cast<const unsigned int *>(entry+4*2));
        LOG(1, "4th instr is " << (int)*reinterpret_cast<const unsigned int *>(entry+4*3));

#if 0
        if((*reinterpret_cast<const unsigned char *>(entry) & 0x9f) == 0x90) {
            address_t pltAddress = header->sh_addr + i;
            unsigned int bytes = *reinterpret_cast<const unsigned int *>(entry);
            address_t value = ((bytes >> 8) & ((1 << 19) - 1))  // bits 5-24
                | ((bytes >> 1) & ((1 << 2) - 1));  // bits 29-31
            value <<= 12;
            LOG(1, "VALUE might be " << value);
            address_t value = *reinterpret_cast<const unsigned int *>(entry + 2)
                + (pltAddress + 2+4);  // target is RIP-relative
            LOG(1, "PLT value would be " << value);
            Reloc *r = registry->find(value);
            if(r) {
                LOG(1, "Found PLT entry at " << pltAddress << " -> ["
                    << r->getSymbol()->getName() << "]");
                entryMap[pltAddress] = new PLTEntry(
                    pltAddress, r->getSymbol());
            }
        }
#endif
    }
#endif
}

PLTEntry *PLTSection::find(address_t address) {
    auto it = entryMap.find(address);
    return (it != entryMap.end() ? (*it).second : nullptr);
}

Reloc *PLTRegistry::find(address_t address) {
    auto it = registry.find(address);
    return (it != registry.end() ? (*it).second : nullptr);
}

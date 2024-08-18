#include "Elf32.h"

#include "LibC.h"

static inline BOOLEAN Elf32CheckFile(Elf32_Ehdr* ehdr){
    return ehdr->e_ident[0] == EI_MAG0 && ehdr->e_ident[1] == EI_MAG1 && 
        ehdr->e_ident[2] == EI_MAG2 && ehdr->e_ident[3] == EI_MAG3;
}

BOOLEAN Efl32CheckSupported(IN Elf32_Ehdr* ehdr){
    return ehdr->e_machine == EM_386 && ehdr->e_ident[EI_CLASS] == ELFCLASS32 &&
        ehdr->e_ident[EI_DATA] == ELFDATA2LSB && ehdr->e_version >= EV_CURRENT;
}

BOOLEAN Elf32CheckExecutabel(IN Elf32_Ehdr* ehdr){
    return ehdr->e_type == ET_EXEC || ehdr->e_type == ET_DYN;
}

static inline VOID Elf32GetEhdr(Elf32_Map *map, CHAR8 *file)
{
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*) file;
    if(Elf32CheckFile(ehdr)){
        map->ehdr = ehdr;
    }else{
        map->ehdr = 0;
    }
}

static inline VOID Elf32GetShdr(Elf32_Map *map, CHAR8 *file)
{
    if(map->ehdr->e_shoff){
        map->shdr = (Elf32_Shdr*)(file + map->ehdr->e_shoff);
        if(map->ehdr->e_shnum){
            map->nshdr = map->ehdr->e_shnum;
        }else{
            map->nshdr = map->shdr->sh_size;
        }
    }else{
        map->shdr = 0;
        map->nshdr = 0;
    }
}

static inline VOID ELF32GetPhdr(Elf32_Map *map, CHAR8 *file)
{
    if(map->ehdr->e_phoff){
        map->phdr = (Elf32_Phdr*)(file + map->ehdr->e_phoff);
        if(map->ehdr->e_phnum != PN_XNUM){
            map->nphdr = map->ehdr->e_phnum;
        }else{
            map->nphdr = map->shdr->sh_info;
        }
    }else{
        map->phdr = 0;
        map->nphdr = 0;
    }
}

static inline VOID Elf32GetStr(Elf32_Map *map, CHAR8 *file)
{
    if(map->ehdr->e_shstrndx != SHN_UNDEF){
        if(map->ehdr->e_shstrndx != SHN_XINDEX){
            map->str = file + map->shdr[map->ehdr->e_shstrndx].sh_offset;
        }else{
            map->str = file + map->shdr[map->shdr->sh_link].sh_offset;
        }
    }else{
        map->str = 0;
    }
}

BOOLEAN Elf32GetMap(OUT Elf32_Map *map, IN CHAR8 *file)
{
    Elf32GetEhdr(map, file);
    if(map->ehdr){
        Elf32GetShdr(map, file);
        if(map->shdr){
            ELF32GetPhdr(map, file);
            Elf32GetStr(map, file);
        }
        return TRUE;
    }
    return FALSE;
}

CHAR8* Elf32GetStrSection(IN Elf32_Map* map, IN UINT32 shindx){
    CHAR8* file = (CHAR8*) map->ehdr;
    return file + map->shdr[shindx].sh_offset;
}

VOID* Elf32GetTable(IN Elf32_Map* map, IN Elf32_Shdr* shdr){
    CHAR8* file = (CHAR8*) map->ehdr;
    return (Elf32_Sym*) (file + shdr->sh_offset);
}

Elf32_Shdr* Elf32GetSHeader(IN Elf32_Map* map, IN UINT32 shindx){
    return &map->shdr[shindx];
}

UINT32 Elf32GetNumEntries(IN Elf32_Shdr* shdr){
    return shdr->sh_size / shdr->sh_entsize;
}

BOOLEAN Elf32LoadFile(IN Elf32_Map* map, IN VOID* offset){
    CHAR8* org = offset;
    map->org = org;

    CHAR8* file = (CHAR8*) map->ehdr;

    // copy program sections
    Elf32_Phdr* phdr = map->phdr;
    for (UINT32 i = 0; i < map->nphdr; i++)
    {
        if(phdr->p_type == PT_LOAD){

            // copy data
            MemCopy(file + phdr->p_offset, org + phdr->p_vaddr, MIN(phdr->p_memsz, phdr->p_filesz));
            if(phdr->p_memsz > phdr->p_filesz){
                MemSet(org + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
            }
        }
        phdr ++ ;
    }

    // set bss sections to zero
    Elf32_Shdr* shdr = map->shdr;
    for (UINT32 i = 0; i < map->nshdr; i++)
    {
        if(shdr->sh_type == SHT_NOBITS){

            // set region to 0
            MemSet((CHAR8*)org + shdr->sh_addr, 0, shdr->sh_size);
        }

        shdr ++;
    }

    return 1;
}
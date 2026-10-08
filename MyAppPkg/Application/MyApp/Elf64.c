#include "Elf64.h"

#include "LibC.h"

static inline BOOLEAN Elf64CheckFile(Elf64_Ehdr* ehdr){
    return ehdr->e_ident[0] == EI_MAG0 && ehdr->e_ident[1] == EI_MAG1 && 
        ehdr->e_ident[2] == EI_MAG2 && ehdr->e_ident[3] == EI_MAG3;
}

BOOLEAN Elf64CheckSupported(IN Elf64_Ehdr* ehdr){
    return ehdr->e_ident[EI_CLASS] == ELFCLASS64 &&
        ehdr->e_ident[EI_DATA] == ELFDATA2LSB && ehdr->e_version >= EV_CURRENT;
}

BOOLEAN Elf64CheckExecutabel(IN Elf64_Ehdr* ehdr){
    return ehdr->e_type == ET_EXEC || ehdr->e_type == ET_DYN;
}

static inline VOID Elf64GetEhdr(Elf64_Map *map, CHAR8 *file)
{
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*) file;
    if(Elf64CheckFile(ehdr)){
        map->ehdr = ehdr;
    }else{
        map->ehdr = 0;
    }
}

static inline VOID Elf64GetShdr(Elf64_Map *map, CHAR8 *file)
{
    if(map->ehdr->e_shoff){
        map->shdr = (Elf64_Shdr*)(file + map->ehdr->e_shoff);
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

static inline VOID ELF64GetPhdr(Elf64_Map *map, CHAR8 *file)
{
    if(map->ehdr->e_phoff){
        map->phdr = (Elf64_Phdr*)(file + map->ehdr->e_phoff);
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

static inline VOID Elf64GetStr(Elf64_Map *map, CHAR8 *file)
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

BOOLEAN Elf64GetMap(OUT Elf64_Map *map, IN CHAR8 *file)
{
    Elf64GetEhdr(map, file);
    if(map->ehdr){
        Elf64GetShdr(map, file);
        if(map->shdr){
            ELF64GetPhdr(map, file);
            Elf64GetStr(map, file);
        }
        return TRUE;
    }
    return FALSE;
}

CHAR8* Elf64GetStrSection(IN Elf64_Map* map, IN UINT64 shindx){
    CHAR8* file = (CHAR8*) map->ehdr;
    return file + map->shdr[shindx].sh_offset;
}

VOID* Elf64GetTable(IN Elf64_Map* map, IN Elf64_Shdr* shdr){
    CHAR8* file = (CHAR8*) map->ehdr;
    return (Elf64_Sym*) (file + shdr->sh_offset);
}

Elf64_Shdr* Elf64GetSHeader(IN Elf64_Map* map, IN UINT64 shindx){
    return &map->shdr[shindx];
}

UINT64 Elf64GetNumEntries(IN Elf64_Shdr* shdr){
    return shdr->sh_size / shdr->sh_entsize;
}

BOOLEAN Elf64LoadFile(IN Elf64_Map* map, IN VOID* offset){
    CHAR8* org = offset;
    map->org = org;

    CHAR8* file = (CHAR8*) map->ehdr;

    // copy program sections
    Elf64_Phdr* phdr = map->phdr;
    for (UINT64 i = 0; i < map->nphdr; i++)
    {
        if(phdr->p_type == PT_LOAD){
            // copy data
            MemCopy(file + phdr->p_offset, org + phdr->p_paddr, MIN(phdr->p_memsz, phdr->p_filesz));
            if(phdr->p_memsz > phdr->p_filesz){
                MemSet(org + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
            }
        }
        phdr ++ ;
    }

    return 1;
}
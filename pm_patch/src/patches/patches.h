#ifndef __PATCHES_H
#define __PATCHES_H

typedef struct _STR_REPLACE_ENTRY{
    int offset;
    char* str;
}StrReplaceEntry,*PStrReplaceEntry;

void Patch_SetWindowedMode(void);
void Patch_QCTest(void);
void Patch_TrackballMenu(void);
void Patch_DevTest(void);
void Patch_Autoplay(void);
void Patch_SkipWarning(void);
void Patch_FilesystemPaths(void);
void Patch_OSSSoundFix(void);
void Patch_Language(void);
#endif

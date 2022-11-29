#include <stdio.h>
#include <sys/mman.h>


#define PM_SYSTEM_STATUS_SHUTDOWN 4
#define ADDR_SYSTEM_STATUS 0x08429880
#define ADDR_SYSTEM_END 0x08056130
void (*SystemEnd)(void) = (void*)ADDR_SYSTEM_END;

void Shutdown(void){
    int* PM_SystemStatus = (int*)ADDR_SYSTEM_STATUS;    
    *PM_SystemStatus = PM_SYSTEM_STATUS_SHUTDOWN;
    SystemEnd();
}

void PrintHex(unsigned char* data, unsigned int len) {
        unsigned int i;
	for (i = 0; i < len; i++) {
		printf("%02X", data[i]);
	}
	printf("\n");
}

void UnprotectPage(int addr){    
    mprotect((void*)(addr-(addr%0x1000)),0x1000,PROT_READ|PROT_WRITE|PROT_EXEC);
}

void PatchCall(void* call_address, void* target_address){
    int call_delta = (int)target_address - (int)(call_address+5);
    UnprotectPage((int)call_address);
    *(int*)(call_address+1) = call_delta;
}

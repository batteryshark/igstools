#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/init.h> /* Needed for the macros */ 
#include <linux/kernel.h> /* Needed for pr_info() */ 
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <asm/io.h>
#include <asm/irq.h>



struct cardbus_info
{
  unsigned int bus;
  unsigned int dev_fn;
  unsigned int status_cmd;
  unsigned int exca_base_address;
  unsigned int mem_base_0;
  unsigned int mem_base_1;
  unsigned int mem_limit_0;
  unsigned int mem_limit_1;
  unsigned int io_base_0;
  unsigned int io_base_1;
  unsigned int io_limit_0;
  unsigned int io_limit_1;
  unsigned short bridge_ctrl;
  unsigned int bridge_interrupt_pin;
  unsigned int bridge_line;
  unsigned int pccard_base_address;
  unsigned int sys_ctrl;
  unsigned short card_ctrl;
  unsigned char dev_ctrl;
  unsigned char diag_reg;
  unsigned int v_exca_base_address;
  unsigned int v_exca_mem_base;
};

// Useful Macros
#define LOWORD(l) ((unsigned short)(l))
#define HIWORD(l) ((unsigned short)(((unsigned int)(l) >> 16) & 0xFFFF))
#define LOBYTE(w) ((unsigned char)(w))
#define HIBYTE(w) ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))
#define BYTEn(x, n)   (*((unsigned char*)&(x)+n))
#define BYTE1(x)   BYTEn(x,  1)  

// Globals
static unsigned char bINT;
static int base = 0x378;
static int major = 0;
static int pccard_base = 0;
static int irq = 0xFFFFFFFF;
static int pccard_irq = 0xFFFFFFFF;
static int m_ulReadMissTimes = 0;
static struct cardbus_info CBI;
static char* pcCmdPort;
static char* pcIntPort;
static char* pcClrIntPort;
static unsigned char* ulMemBase;
static char* pcClrExCAIntPort;
static unsigned char ucCmdPortVal;
static unsigned char m_bCmdPortValDiff;
static unsigned char m_bInterruptError;
static unsigned char m_bCmdPortVal;
static unsigned long m_ulIntWhileTimes;
static unsigned char m_bCmdPortValRead;
static unsigned long m_ulCnt;
static unsigned long m_ulLinuxReadCnt;
static unsigned long m_ulIntAnother;
static unsigned char m_bIntComeIn;
static unsigned char m_bReadComeIn;
static unsigned char m_bWriteComeIn;
static unsigned long m_ulA27IntCnt;


// This doesn't seem to be a thing anymore...
int pcibios_present(void){return 1;}

void pccard_kernelprobe(void){

  int irq;
  int probe_attempt;
  for(probe_attempt = 0; probe_attempt < 5; probe_attempt++){
	irq = probe_irq_on();
    *(unsigned char *)pcCmdPort = (probe_attempt - 0x79) & 0xFF;
    *(unsigned char *)pcIntPort = (probe_attempt - 0x79) & 0xFF;
    __const_udelay(0x3EE68);
    pccard_irq = probe_irq_off(irq);
    if ( !pccard_irq ){
		printk("[A27::pccard_kernelprobe] <6>pccard: no irq reported by probe\n");
		pccard_irq = -1;
    }else{
		printk("[A27::pccard_kernelprobe] pccard_irq set to: %d\n",pccard_irq);
		break;
	}
  }
  if(pccard_irq < 0){
	  printk("[A27::pccard_kernelprobe] short: probe failed %i times, giving up\n", probe_attempt);
  }
}

int pccard_interrupt(void){

  int result = ulMemBase + 0x1FFFF;
  if ( *(unsigned char *)(ulMemBase + 0x1FFFF) == 'R'
    && *(unsigned char *)(ulMemBase + 0x1FFFE) == 'F'
    && *(unsigned char *)(ulMemBase + 0x1FFFD) == '4' )
  {
    *(unsigned char *)(ulMemBase + 0x1FFFF) = 0;
    *(unsigned char *)(result - 1) = 0;
    *(unsigned char *)(result - 2) = 0;
    ucCmdPortVal = *(unsigned char *)pcCmdPort;
    m_bCmdPortValDiff = 0;
    m_ulReadMissTimes = 0;
    m_bInterruptError = 0;
    m_ulIntWhileTimes = 0;
    if ((unsigned char)++m_bCmdPortVal > 0xEFu ){
		m_bCmdPortVal = 1;
	}
    char v1 = *(unsigned char *)pcCmdPort;
    if ( *(unsigned char *)pcCmdPort == 0xF1 ){
		printk("[A27::pccard_interrupt] Before something that might panic");
		printk("[A27::pccard_interrupt] <int>debug code=%d,%d,%d\n", *(unsigned char *)pcCmdPort+0x1F800,*(unsigned char *)pcCmdPort+0x1F801, *(unsigned char *)pcCmdPort+0x1F802);
		printk("[A27::pccard_interrupt] After something that might panic");
	}
      
    if ( m_bCmdPortVal != v1 ){
      printk("[A27::pccard_interrupt] <int>command port value error!!\n");
      m_bCmdPortValDiff = 1;
    }
    m_bCmdPortValRead = v1;
    *(unsigned char *)pcCmdPort = m_bCmdPortVal;
    bINT = -1;
    return 0;
	// __wake_up(1, 1);
	// Not sure 
  }else{
	      printk("[A27::pccard_interrupt] No RF4 Identifier on memory\n");  
  }
  return result;
}

// Prototypes
static int pccard_open(struct inode *inode, struct file *file);
static int pccard_release(struct inode* inode, struct file* filep){return 0;}

int pccard_bDataSend(void){
  int v0; // edx
  int i; // edx
  int j; // eax

  v0 = ulMemBase + 0x1FFFF;
  *(unsigned char *)(ulMemBase + 0x1FFFF) = 0;
  *(unsigned char *)(v0 - 1) = 0;
  *(unsigned char *)(v0 - 2) = 0;
  m_ulReadMissTimes = 0;
  m_bCmdPortValDiff = 0;
  m_bInterruptError = 0;
  m_ulIntWhileTimes = 0;
  while ( *(unsigned char *)pcCmdPort <= 0xEFu )
  {
    for ( i = 0; i <= 99; ++i )
    {
      for ( j = 4; j >= 0; --j )
        ;
    }
    ++m_ulIntWhileTimes;
  }
  m_bCmdPortValRead = *(unsigned char *)pcCmdPort;
  *(unsigned char *)pcCmdPort = m_bCmdPortVal;
  return 1;
}

ssize_t pccard_i_read(struct file *file, char *output_buffer, size_t count, loff_t *ppos)
{
  unsigned char result; // bl
  printk("[A27::pccard_i_read] Amount: %d\n",count);
  result = 0;
  if ( (unsigned char*)pccard_bDataSend() ){
    ++m_ulLinuxReadCnt;
    result = 0xF3;
    if ( !m_bCmdPortValDiff )
    {
      result = 0xF4;
      if ( !m_bInterruptError )
        result = m_bCmdPortValRead;
    }

    *(unsigned char *)(ulMemBase + 65) = 100;

    copy_to_user(output_buffer, ulMemBase, *(unsigned int *)ulMemBase + 132);

    *(unsigned int *)ulMemBase = 0;
    bINT = 0;
  }else{
	  printk("[A27::pccard_i_read] Error - Data Not Sent wtf\n");
  }
  return result;
}

int pccard_i_write(struct file *file, const char *ubuff, size_t count, loff_t *off){
  int v4; // ecx
  char v5; // al
  int v6; // ebx
  int v7; // edx
  int v8; // ecx
  bool v9; // zf
  int v10; // edx
  int k; // eax
  int j; // eax
  int i; // edx
  unsigned char v15; // [esp+Eh] [ebp-Ah]
  unsigned char v16; // [esp+Fh] [ebp-9h]
  printk("[A27:: pcicard_i_write] %d bytes, Offset: %d",count,*off);
  v4 = ulMemBase + 0x1FFFF;
  if ( *off == 254 )
  {
    v5 = *(unsigned char *)off;
    m_ulIntAnother = 0;
    m_ulLinuxReadCnt = 0;
    m_bIntComeIn = 0;
    m_bReadComeIn = 0;
    m_bWriteComeIn = 0;
    bINT = 0;
    m_bCmdPortVal = 1;
    *(unsigned char *)pcCmdPort = v5;
    m_ulA27IntCnt = 0;
    *(unsigned int *)(v4 - 6) = 0;
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>start(%d)\n", *(unsigned int *)off);
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>==============================\n");
  }
  else
  {
    if ( (unsigned char)++m_bCmdPortVal > 0xEFu )
      m_bCmdPortVal = 1;
    v7 = pcCmdPort;
    *(unsigned char *)pcCmdPort = m_bCmdPortVal;
    copy_from_user(ulMemBase, ubuff, count);
  }
  v8 = jiffies;
  *(unsigned char *)pcIntPort = 1;
  if ( (unsigned char)++m_bCmdPortVal > 0xEFu )
    m_bCmdPortVal = 1;
  v16 = *(unsigned char *)pcCmdPort;
  v15 = *(unsigned char *)pcCmdPort;
  v9 = 1;
  v6 = 1;
  while ( 1 )
  {
    if ( !v9 )
    {
      for ( i = 499; i >= 0; --i )
        ;
      goto LABEL_21;
    }
    if ( v16 == m_bCmdPortVal )
      return v6;
    v10 = 0;
    if ( v16 == 0xFD )
      break;
    do
    {
      for ( j = 9; j >= 0; --j )
        ;
      ++v10;
    }
    while ( v10 <= 99 );
LABEL_21:
    if ( (unsigned int)(jiffies - v8) > 0x3E8 )
    {
      printk("<write>f_pos=%u,", *(unsigned int *)off);
      printk("time out(%d,%d,%d)\n", (unsigned char)m_bCmdPortVal, v16, v15);
      return 0;
    }
    v16 = v15;
    v15 = *(unsigned char *)pcCmdPort;
    v9 = v16 == *(unsigned char *)pcCmdPort;
  }
  do
  {
    for ( k = 9; k >= 0; --k )
      ;
    ++v10;
  }
  while ( v10 <= 9999 );
  printk("<write>f_pos=%u", *(unsigned int *)off);
  printk("re-send int to asic27\n");
  *(unsigned char *)pcIntPort = 1;
  return 0;
}

static struct file_operations pccard_i_fops = {
	read: pccard_i_read,
    write: pccard_i_write,
	open: pccard_open,
    release: pccard_release
};

static int pccard_open(struct inode *inode, struct file *file){
	printk("[A27::pccard_open]\n");
	sizeof(struct inode)
  	//if ( *((char *)inode + 0x40) < 0 ) 
	//if(inode->i_rdev < 0)
	
	file->f_op = &pccard_i_fops;
  return 0;
}

int GetIntState(void){
	printk("[A27::GetIntState]\n");	
	int result; // eax
	asm("cli");
	//_disable();
	result = bINT;
	bINT = 0;
	asm("sti");
	//_enable();
	return result;

	/*
	push ebp
	mov  ebp, esp
	cli
	mov  eax, ds:_bINT
	mov  ds:_bINT, 0
	sti
	pop  ebp
	retn

	*/
}

#define PCI_ANY_ID (~0)
struct pci_dev *pci_find_slot(unsigned int bus, unsigned int devfn){
	struct pci_dev *dev = NULL;

	while ((dev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev)) != NULL) {
		if (dev->bus->number == bus && dev->devfn == devfn) {
			pci_dev_put(dev);
			return dev;
		}
	}
	return NULL;
}

void GetCardBusInfo(void){
		printk("[A27::GetCardBusInfo]\n");
	struct pci_dev *slot = (struct pci_dev *)pci_find_slot(CBI.bus, CBI.dev_fn);
	int pci_dword;
	pci_read_config_dword(slot, 8, &pci_dword);
	// Check to ensure this is a PCCARD
	if((pci_dword & 0xFFFF0000) != 0x6070000){
		printk("[A27::GetCardBusInfo] Error: not PCCARD\n");
	  return;
	}

	pci_read_config_dword(slot, 4, &CBI.status_cmd);
	pci_read_config_dword(slot, 16, &pci_dword);
	CBI.exca_base_address = pci_dword;
	pci_read_config_dword(slot, 28, &CBI.mem_base_0);
	pci_read_config_dword(slot, 36, &CBI.mem_base_1);
	pci_read_config_dword(slot, 32, &CBI.mem_limit_0);
	pci_read_config_dword(slot, 40, &CBI.mem_limit_1);
	pci_read_config_dword(slot, 44, &CBI.io_base_0);
	pci_read_config_dword(slot, 52, &CBI.io_base_1);
	pci_read_config_dword(slot, 48, &CBI.io_limit_0);
	pci_read_config_dword(slot, 56, &CBI.io_limit_1);
	pci_read_config_dword(slot, 60, &pci_dword);
	int tmpval = pci_dword;
	CBI.bridge_line = pci_dword & 0xFF;
	CBI.bridge_ctrl = HIWORD(tmpval);
	CBI.bridge_interrupt_pin = BYTE1(tmpval);
	pci_read_config_dword(slot, 68, &CBI.pccard_base_address);
	pci_read_config_dword(slot, 128, &CBI.sys_ctrl);
	pci_read_config_dword(slot, 144, &pci_dword);
	unsigned short tmpval_2 = BYTE1(pci_dword);
	*(unsigned short *)&CBI.dev_ctrl = HIWORD(pci_dword);
	CBI.card_ctrl = tmpval_2;

}

void SetCardBusInfo(void){
	printk("[A27::SetCardBusInfo]\n");	
	int pci_read_dword = 0;
	struct pci_dev* slot = pci_find_slot(CBI.bus, CBI.dev_fn);
	pci_read_config_dword(slot, 8, &pci_read_dword);
	if ( (pci_read_dword & 0xFFFF0000) == 0x6070000 ) {
		pci_write_config_word(slot, 4, 7);
		pci_write_config_dword(slot, 0x2C, 0x10000);
		pci_write_config_dword(slot, 0x10, 0xCB000008);
		CBI.exca_base_address = 0xCB000000;
		// FIX: IOREMAP DOESNT USE FLAGS, MAYBE OLD INTERNAL VERSION DOES - PAGE_PCD is used on nocache
		CBI.v_exca_base_address = ioremap_nocache(0xCB000000, 0x1000);
		pci_write_config_dword(slot, 0x1C, CBI.exca_base_address + 0x2000008);
		pci_write_config_dword(slot, 0x20, CBI.exca_base_address + 0x3000008);
		CBI.v_exca_mem_base = ioremap_nocache(CBI.exca_base_address + 0x2000000, 0xC100000);
		pci_write_config_dword(slot, 0x8C, 0x110112);
		pci_write_config_dword(slot, 0xAC, 0);
		pci_read_config_dword(slot, 0x90, &pci_read_dword);
		pci_write_config_dword(slot, 0x90, pci_read_dword & 0xFFF9FFFF);
	}else{
		printk("[A27::SetCardBusInfo] Error - Not PCCard\n");	
	}
}

void SetExCAInfo(void){
	printk("[A27::SetExCAInfo]\n");
	unsigned char *exca_buffer = (unsigned char *)CBI.v_exca_base_address;

	exca_buffer[0x806] &= 0xFCu;
	exca_buffer[0x805] |= 4;
	exca_buffer[0x810] = 0;
	exca_buffer[0x811] = 0x80;
	exca_buffer[0x812] = 0xFF;
	exca_buffer[0x813] = 15;
	exca_buffer[0x814] = 0;
	exca_buffer[0x815] = 0;
	exca_buffer[0x840] = HIBYTE(CBI.mem_base_0);
	exca_buffer[0x818] = 0;
	exca_buffer[0x819] = 0x88;
	exca_buffer[0x81A] = 0xFF;
	exca_buffer[0x81B] = 0xF;
	exca_buffer[0x81C] = 0;
	exca_buffer[0x81D] = 8;
	exca_buffer[0x841] = HIBYTE(CBI.mem_base_0);
	exca_buffer[0x806] = 3;
	exca_buffer[0x802] = 0x90;
}

void DisplayCardInfo(void){
	printk("[A27::DisplayCardInfo]\n");
	printk("BUS %d,DevNUM %d,Status Command 0x%08x\n", CBI.bus, CBI.dev_fn, CBI.status_cmd);
	printk("ExCA Base Address %08X\n", CBI.exca_base_address);
	printk("Memory Base0  %08X, Base1  %08X\n", CBI.mem_base_0, CBI.mem_base_1);
	printk("Memory Limit0 0x%08x, Limit1 0x%08x\n", CBI.mem_limit_0, CBI.mem_limit_1);
	printk("IO Base0  %08X, Base1  %08X\n", CBI.io_base_0, CBI.io_base_1);
	printk("IO Limit0 0x%08x, Limit1 0x%08x\n", CBI.io_limit_0, CBI.io_limit_1);
	printk("Bridge Ctrl %4x,Interrupt Pin %2x,Line %2x\n", CBI.bridge_ctrl, CBI.bridge_interrupt_pin, CBI.bridge_line);
	printk(
	"PCCARD Base Address %08x,Sys Ctrl %08X,Card Ctrl %02X,Dev Ctrl %02X\n",
	CBI.pccard_base_address,
	CBI.sys_ctrl,
	CBI.card_ctrl,
	CBI.dev_ctrl);
	printk("Diag reg 0x%02x\n", CBI.diag_reg);
	printk("Virtual ExCA Base Address %08XM\n", CBI.v_exca_base_address);
}

void ResetExCA(void){
	printk("[A27::ResetExCA]\n");
	char exca_value = *(unsigned char *)(CBI.v_exca_base_address + 0x803);
	exca_value |= 0x40;
	int crap = jiffies;
	exca_value &= 0xBF;

	*(unsigned char *)(CBI.v_exca_base_address + 0x803) = exca_value;
}

void TestPCICardBusInit(void){
	printk("[A27::TestPCICardBusInit]\n");
	GetCardBusInfo();
	SetCardBusInfo();
	GetCardBusInfo();
	DisplayCardInfo();
	printk("ExCA %p,MEM %p\n", CBI.exca_base_address, CBI.mem_base_0);
	printk("ExCA %p,MEM %p\n", CBI.v_exca_base_address, CBI.v_exca_mem_base);
	SetExCAInfo();

	pcCmdPort = CBI.v_exca_mem_base + 0x800000;
	pcIntPort = CBI.v_exca_mem_base + 0xA00000;
	pcClrIntPort = CBI.v_exca_mem_base + 0xC00000;
	ulMemBase = CBI.v_exca_mem_base;
	pcClrExCAIntPort = CBI.v_exca_base_address + 0x804;
	ucCmdPortVal = *(unsigned char *)(CBI.v_exca_mem_base + 0x800000);
	bINT = 0;
}

int FindPCIDevice(void){
	printk("[A27::FindPCIDevice]\n");	
	struct pci_dev *slot;
	int dwRead; 
	int tmpval;
	int result = -1;
	int current_bus;
	int current_dev_fn;
	memset(&CBI, 0, sizeof(CBI));
	if (pcibios_present()){
		for ( current_bus = 0; current_bus <= 4; ++current_bus ){
			for ( current_dev_fn = 0; current_dev_fn <= 255; ++current_dev_fn ){
				slot = (struct pci_dev *)pci_find_slot(current_bus, current_dev_fn);
				if ( slot ){
					pci_read_config_dword(slot, 0, &dwRead);
					tmpval = dwRead;
					if ( dwRead != -1 ){
						printk("[A27::FindPCIDevice] BUS %d,DEV %2d DID VID Reg contain :%08X\n", current_bus, current_dev_fn, dwRead);
						pci_read_config_dword(slot, 8, &dwRead);
						if ((dwRead & 0xFFFF0000) == 0x6070000){ // CardBUS Class
							printk("Found CardBUS CLASS on %d\n", current_dev_fn);
							if (tmpval == 0xAC50104C ){  // PCI1410 PC card cardBus Controller
								printk("[A27::FindPCIDevice] Found CardBUS device on:%d Class %08X\n", current_dev_fn, dwRead);
								result = current_dev_fn;
								CBI.dev_fn = current_dev_fn;
								CBI.bus = current_bus;
								return (result == -1) - 1;
							}
						}
					}
				}
			}
		}
		return (result == -1) - 1;
	}else{
		printk("[A27::FindPCIDevice] No PCI bios present\n");
		return 0;
	}
}

void DisplayExCARegisters(void){
  unsigned int v_exca_base_Address; // esi
  int i; // ebx
  int j; // ebx
  int result; // eax
  int v5; // [esp+0h] [ebp-24h]
  int v6; // [esp+0h] [ebp-24h]
  int v7; // [esp+4h] [ebp-20h]
  int v8; // [esp+4h] [ebp-20h]

  v_exca_base_Address = CBI.v_exca_base_address;
  printk("[A27::DisplayExCARegisters] CardBus Socket Registers\n");
  for ( i = 0; i <= 32; ++i )
  {
    if ( i > 0 && !(i % 10) )
      printk("\n");
    v7 = *(unsigned char *)(i + v_exca_base_Address);
    v5 = i;
    printk("[A27::DisplayExCARegisters] %03X:%02X ", v5, v7);
  }
  printk("[A27::DisplayExCARegisters] Display ExCA Registers\n");
  for ( j = 0; j <= 68; ++j )
  {
    if ( j > 0 && !(j % 10) )
      printk("\n");
    v8 = *(unsigned char *)(j + v_exca_base_Address + 0x800);
    v6 = j + 0x800;
    printk("[A27::DisplayExCARegisters] %03X:%02X ", v6, v8);
  }

}



int pccard_init(void){

	pccard_base = base;
	pccard_irq = irq;
	m_ulReadMissTimes = 0;
	pccard_i_fops = 0;
	if (FindPCIDevice()){
		TestPCICardBusInit();
		DisplayExCARegisters();
		ResetExCA();
	}else{
		printk("[A27::pccard_init] Error - PCI Card Not Found!\n");
		return;
	}
	int n_major = register_chrdev(major, "pccard", &pccard_i_fops);
	if ( n_major >= 0 ) {
		if ( !major ){
			major = n_major;
			printk("[A27::pccard_init] major set to %x\n", major);
		}      
		
		if (CBI.bridge_line){
			pccard_irq = CBI.bridge_line;
		}
		  
		if (pccard_irq < 0){
			printk("[A27::pccard_init] IRQ not set - probing...\n");
			pccard_kernelprobe();
		}
		  
		if (pccard_irq >= 0){
			if (request_irq(pccard_irq, (void*)&pccard_interrupt, 0x20000000, "pccard", (void*)&pccard_interrupt)){
				printk("[A27::pccard_init] <6>pccard: can't get assigned irq %i\n", pccard_irq);
				pccard_irq = -1;
				return 0;
			}
			printk("[A27::pccard_init] IRQ set to %d\n", pccard_irq);
		}
		
	}else{
		printk("<6>short: can't get major number\n");
	}
	return 0;
}

void pccard_cleanup(void){


	if ( pccard_irq >= 0 ){
	  free_irq(pccard_irq, pccard_interrupt);
	}
    
	unregister_chrdev(major, "pccard");
	printk("[A27::pccard_cleanup] Unmap ExCA:%08x,MEM:%08x\n", CBI.v_exca_base_address, CBI.v_exca_mem_base);

	if (CBI.v_exca_base_address){
		iounmap(CBI.v_exca_base_address);  
	}

	if (CBI.v_exca_mem_base){
		iounmap(CBI.v_exca_mem_base);  
	}

	return;
}


module_init(pccard_init); 
module_exit(pccard_cleanup); 

MODULE_LICENSE("GPL");
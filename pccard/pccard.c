// Replacement IGS PCCARD Kernel Module

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/pci.h>

// linux/kdev_t.h
// we might need this.
#include "defs.h"

static struct cardbus_info{
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
}CBI;

static unsigned long ulMemBase = 0;
static unsigned long m_ulReadMissTimes = 0;
static unsigned long m_ulLinuxReadCnt = 0;
static int pccard_major = 0;
static unsigned char bINT = 0;
static unsigned char m_bCmdPortValDiff = 0;
static unsigned char m_bInterruptError = 0;
static unsigned long m_ulIntWhileTimes = 0;
static char* pcCmdPort = NULL;
static char* pcIntPort = NULL;
static unsigned long m_ulIntAnother = 0;
static unsigned char m_bIntComeIn = 0;
static unsigned char m_bReadComeIn = 0;
static unsigned char m_bWriteComeIn = 0;
static unsigned long m_ulA27IntCnt = 0;
static unsigned char m_bCmdPortValRead = 0;
static unsigned char m_bCmdPortVal = 0;
static char* pcClrIntPort = NULL;
static char* pcClrExCAIntPort = NULL;
static unsigned char ucCmdPortVal = 0;

static int pccard_open(struct inode *inode, struct file *file);


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


static int pccard_i_write(struct file *file, const char *ubuff, size_t count, loff_t *off){

  int v4; // ecx
  char v5; // al
  int v6; // ebx
  int v7; // ecx
  bool v8; // zf
  int v9; // edx
  int k; // eax
  int j; // eax
  int i; // edx
  unsigned char v14; // [esp+Eh] [ebp-Ah]
  unsigned char v15; // [esp+Fh] [ebp-9h]
  printk("[A27::pccard_i_write]\n");
  *off = file->f_pos;
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
    *(unsigned char *)pcCmdPort = m_bCmdPortVal;
    copy_from_user((void*)ulMemBase, ubuff, count);
  }
  v7 = jiffies;
  *(unsigned char *)pcIntPort = 1;
  if ( (unsigned char)++m_bCmdPortVal > 0xEFu )
    m_bCmdPortVal = 1;
  v15 = *(unsigned char *)pcCmdPort;
  v14 = *(unsigned char *)pcCmdPort;
  v8 = 1;
  v6 = 1;
  while ( 1 )
  {
    if ( !v8 )
    {
      for ( i = 499; i >= 0; --i )
        ;
      goto LABEL_21;
    }
    if ( v15 == m_bCmdPortVal )
      return v6;
    v9 = 0;
    if ( v15 == 0xFD )
      break;
    do
    {
      for ( j = 9; j >= 0; --j )
        ;
      ++v9;
    }
    while ( v9 <= 99 );
LABEL_21: // 1000 Cycles is a lot
    if ( (unsigned int)(jiffies - v7) > 1000 )
    {
      printk("<write>f_pos=%u,", *(unsigned int *)off);
      printk("time out(%d,%d,%d)\n", (unsigned char)m_bCmdPortVal, v15, v14);
      return 0;
    }
    v15 = v14;
    v14 = *(unsigned char *)pcCmdPort;
    v8 = v15 == *(unsigned char *)pcCmdPort;
  }
  do
  {
    for ( k = 9; k >= 0; --k )
      ;
    ++v9;
  }
  while ( v9 <= 9999 );
  printk("<write>f_pos=%u", *(unsigned int *)off);
  printk("re-send int to asic27\n");
  *(unsigned char *)pcIntPort = 1;
  return 0;
}

static ssize_t pccard_i_read(struct file *file, char *output_buffer, size_t count, loff_t *ppos){
  unsigned char return_val = 0;
  printk("[A27::pccard_i_read]\n");
  if (pccard_bDataSend()){
    ++m_ulLinuxReadCnt;
    return_val = 0xF3;
    if ( !m_bCmdPortValDiff )
    {
      return_val = 0xF4;
      if ( !m_bInterruptError )
        return_val = m_bCmdPortValRead;
    }
    *(unsigned char *)(ulMemBase + 0x41) = 0x64;
    copy_to_user(output_buffer, (void*)ulMemBase, *(unsigned int *)ulMemBase + 0x84);
    *(unsigned int *)ulMemBase = 0;
    bINT = 0;
  }
  return return_val;
}

static int pccard_release(struct inode* inode, struct file* file){return 0;}

static loff_t pccard_seek(struct file * file, loff_t offset, int whence){
file->f_pos = offset;
return file->f_pos;
}

static struct file_operations pccard_i_fops = {
  llseek: pccard_seek,
    read: pccard_i_read,
    write: pccard_i_write,
    open: pccard_open,
    release: pccard_release
};

static int pccard_open(struct inode *inode, struct file *file){
	printk("[A27::pccard_open]\n");

	if(MINOR(inode->i_rdev)){
        file->f_op = &pccard_i_fops;
    }

  return 0;
}

// This doesn't seem to be a thing anymore...
int pcibios_present(void){return 1;}

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
int FindPCIDevice(void){
	struct pci_dev *slot;
	int dwRead; 
	int tmpval;
	int result = -1;
	int current_bus;
	int current_dev_fn;
  printk("[A27::FindPCIDevice]\n");	  
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

void GetCardBusInfo(void){
	struct pci_dev *slot = (struct pci_dev *)pci_find_slot(CBI.bus, CBI.dev_fn);
  unsigned short tmpval_2;
	int pci_dword;
  int tmpval;
  printk("[A27::GetCardBusInfo]\n");  
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
	tmpval = pci_dword;
	CBI.bridge_line = pci_dword & 0xFF;
	CBI.bridge_ctrl = HIWORD(tmpval);
	CBI.bridge_interrupt_pin = BYTE1(tmpval);
	pci_read_config_dword(slot, 68, &CBI.pccard_base_address);
	pci_read_config_dword(slot, 128, &CBI.sys_ctrl);
	pci_read_config_dword(slot, 144, &pci_dword);
	tmpval_2 = BYTE1(pci_dword);
	*(unsigned short *)&CBI.dev_ctrl = HIWORD(pci_dword);
	CBI.card_ctrl = tmpval_2;

}

void SetCardBusInfo(void){
	int pci_read_dword = 0;
	struct pci_dev* slot = pci_find_slot(CBI.bus, CBI.dev_fn);
	printk("[A27::SetCardBusInfo]\n");	  
	pci_read_config_dword(slot, 8, &pci_read_dword);
	if ( (pci_read_dword & 0xFFFF0000) == 0x6070000 ) {
		pci_write_config_word(slot, 4, 7);
		pci_write_config_dword(slot, 0x2C, 0x10000);
		pci_write_config_dword(slot, 0x10, 0xCB000008);
		CBI.exca_base_address = 0xCB000000;
		// FIX: IOREMAP DOESNT USE FLAGS, MAYBE OLD INTERNAL VERSION DOES - PAGE_PCD is used on nocache
		CBI.v_exca_base_address = (unsigned int)ioremap_nocache(0xCB000000, 0x1000);
		pci_write_config_dword(slot, 0x1C, CBI.exca_base_address + 0x2000008);
		pci_write_config_dword(slot, 0x20, CBI.exca_base_address + 0x3000008);
		CBI.v_exca_mem_base = (unsigned int)ioremap_nocache(CBI.exca_base_address + 0x2000000, 0xC100000);
		pci_write_config_dword(slot, 0x8C, 0x110112);
		pci_write_config_dword(slot, 0xAC, 0);
		pci_read_config_dword(slot, 0x90, &pci_read_dword);
		pci_write_config_dword(slot, 0x90, pci_read_dword & 0xFFF9FFFF);
	}else{
		printk("[A27::SetCardBusInfo] Error - Not PCCard\n");	
	}
}

void SetExCAInfo(void){
	unsigned char *exca_buffer = (unsigned char *)CBI.v_exca_base_address;
	printk("[A27::SetExCAInfo]\n");
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
	printk("Virtual ExCA Base Address %08X\n", CBI.v_exca_base_address);
}


void TestPCICardBusInit(void){
	printk("[A27::TestPCICardBusInit]\n");
	GetCardBusInfo();
	SetCardBusInfo();
	GetCardBusInfo();
	DisplayCardInfo();
	printk("ExCA %x,MEM %x\n", CBI.exca_base_address, CBI.mem_base_0);
	printk("ExCA %x,MEM %x\n", CBI.v_exca_base_address, CBI.v_exca_mem_base);
	SetExCAInfo();

	pcCmdPort = (char*)CBI.v_exca_mem_base + 0x800000;
	pcIntPort = (char*)CBI.v_exca_mem_base + 0xA00000;
	pcClrIntPort = (char*)CBI.v_exca_mem_base + 0xC00000;
	ulMemBase = CBI.v_exca_mem_base;
	pcClrExCAIntPort = (char*)(CBI.v_exca_base_address + 0x804);
	ucCmdPortVal = *(unsigned char *)(CBI.v_exca_mem_base + 0x800000);
	bINT = 0;
}

void DisplayExCARegisters(void){
  unsigned int v_exca_base_Address; // esi
  int i; // ebx
  int j; // ebx
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

void ResetExCA(void){
	char exca_value = *(unsigned char *)(CBI.v_exca_base_address + 0x803);
	printk("[A27::ResetExCA]\n");  
	exca_value |= 0x40;
	exca_value &= 0xBF;

	*(unsigned char *)(CBI.v_exca_base_address + 0x803) = exca_value;
}

int pccard_init(void){
    m_ulReadMissTimes = 0;
    //pccard_i_fops = 0;

    if (FindPCIDevice()){
        TestPCICardBusInit();
        DisplayExCARegisters();
        ResetExCA();
    }
    pccard_major = register_chrdev(0, "pccard", &pccard_i_fops);
    if(pccard_major >= 0 ){
        printk("\nmajor %x\n", pccard_major);
        return 0;
    }else{
        printk("<6>short: can't get major number\n");
        return pccard_major;
    }
}

void pccard_cleanup(void){
 
  unregister_chrdev(pccard_major, "pccard");
  printk("unmap ExCA:%x,MEM:%x\n", CBI.v_exca_base_address, CBI.v_exca_mem_base);
  if ( CBI.v_exca_base_address ){
    iounmap((void*)CBI.v_exca_base_address);
  }

  if( CBI.v_exca_mem_base ){
    iounmap((void*)CBI.v_exca_mem_base);
  }
}


module_init(pccard_init); 
module_exit(pccard_cleanup); 
MODULE_LICENSE("GPL");
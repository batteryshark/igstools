


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


int __cdecl pccard_i_write(struct file *file, const char *ubuff, size_t count, loff_t *off)
{
  int v4; // ecx
  char v5; // al
  int v6; // ebx
  int v7; // ecx
  bool v8; // zf
  int v9; // edx
  int k; // eax
  int j; // eax
  int i; // edx
  unsigned __int8 v14; // [esp+Eh] [ebp-Ah]
  unsigned __int8 v15; // [esp+Fh] [ebp-9h]

  v4 = ulMemBase + 0x1FFFF;
  if ( *off == 254 )
  {
    v5 = *(_BYTE *)off;
    m_ulIntAnother = 0;
    m_ulLinuxReadCnt = 0;
    m_bIntComeIn = 0;
    m_bReadComeIn = 0;
    m_bWriteComeIn = 0;
    bINT = 0;
    m_bCmdPortVal = 1;
    *(_BYTE *)pcCmdPort = v5;
    m_ulA27IntCnt = 0;
    *(_DWORD *)(v4 - 6) = 0;
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>start(%d)\n", *(_DWORD *)off);
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>==============================\n");
    printk("<write>==============================\n");
  }
  else
  {
    if ( (unsigned __int8)++m_bCmdPortVal > 0xEFu )
      m_bCmdPortVal = 1;
    *(_BYTE *)pcCmdPort = m_bCmdPortVal;
    _generic_copy_from_user(ulMemBase, ubuff, count);
  }
  v7 = jiffies;
  *(_BYTE *)pcIntPort = 1;
  if ( (unsigned __int8)++m_bCmdPortVal > 0xEFu )
    m_bCmdPortVal = 1;
  v15 = *(_BYTE *)pcCmdPort;
  v14 = *(_BYTE *)pcCmdPort;
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
LABEL_21:
    if ( (unsigned int)(jiffies - v7) > 0x3E8 )
    {
      printk("<write>f_pos=%u,", *(_DWORD *)off);
      printk("time out(%d,%d,%d)\n", (unsigned __int8)m_bCmdPortVal, v15, v14);
      return 0;
    }
    v15 = v14;
    v14 = *(_BYTE *)pcCmdPort;
    v8 = v15 == *(_BYTE *)pcCmdPort;
  }
  do
  {
    for ( k = 9; k >= 0; --k )
      ;
    ++v9;
  }
  while ( v9 <= 9999 );
  printk("<write>f_pos=%u", *(_DWORD *)off);
  printk("re-send int to asic27\n");
  *(_BYTE *)pcIntPort = 1;
  return 0;
}
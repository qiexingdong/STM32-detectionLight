//包含头文件
#include "flash.h"
#include "string.h"
#if(USE_BIOS_FLASH==0)//1代表函数继承自BIOS，0代表使用自带函数
//=================内部调用函数声明=====================================
//======================================================================
//函数名称：flash_write_DoubleWord
//函数返回：0-成功 1-失败
//参数说明：addr：目标地址，要求为4的倍数且大于Flash首地址
//              （例如：0x08000004，Flash首地址为0x08000000）
//       data：写入的双字
//功能概要：Flash双字写入操作
//======================================================================
uint8_t flash_write_DoubleWord(uint32_t addr,uint32_t data_l,uint32_t data_h);
//======================================================================
//函数名称：flash_Best
//函数返回：0-成功 1-失败
//参数说明：sect:待写入扇区号
//            offset:待写入数据位置的偏移地址
//            N：待写入数据字节数
//            buf:待写入数据的首地址
//功能概要：首位地址都对齐的情况下的数据写入
//======================================================================
uint8_t flash_Best(uint16_t sect,uint16_t offset,uint16_t N,uint8_t *buf);
//======================================================================
//======================================================================

//=================外部接口函数=========================================
//======================================================================
//函数名称：flash_init
//函数返回：无
//参数说明：无
//功能概要：初始化flash模块
//编程来源：暂无
//======================================================================
void flash_init(void)
{
    //（1）清除之前的编程导致的所有错误标志位
    FLASH->SR &= 0xFFFFFFUL;
    //（2）解锁Flash控制寄存器(CR)
    if((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        FLASH->KEYR = (uint32_t)FLASH_KEY1;
        FLASH->KEYR = (uint32_t)FLASH_KEY2;
    }
    //（3）等待解锁操作成功
    while((FLASH->CR & FLASH_CR_LOCK) != 0U);
    //（4）等待之前最后一个flash操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    //（5）清数据缓冲区
    FLASH->ACR &= ~FLASH_ACR_DCEN_Msk;
    //（6）清闪存即时编程位
    FLASH->CR &= ~FLASH_CR_PG_Msk;
}
//======================================================================
//函数名称：flash_erase
//函数返回：函数执行执行状态：0=正常；1=异常。
//参数说明：sect：目标扇区号（范围取决于实际芯片，例如 STM32L433:0~127,每扇区2KB;
//功能概要：擦除flash存储器的sect扇区
//编程参考：STM32L4Rxxx芯片手册3.3.6主存储器擦除顺序
//======================================================================
uint8_t flash_erase(uint16_t sect)
{
    
    //（1）等待之前最后一个flash操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    //（2）清除之前的编程导致的所有错误标志位
    FLASH->SR &= 0xFFFFFFUL;
    //（3）清闪存即时编程位，将 PER 位置 1，并选择要擦除的页 (PNB)和相关存储区 (BKER)
    FLASH->CR &= ~FLASH_CR_PG;
    FLASH->CR |= FLASH_CR_PER;
    FLASH->CR &= ~FLASH_CR_PNB;
    FLASH->CR |= (uint32_t)(sect << 3u);
    //（4）将 FLASH_CR 寄存器中的 STRT 位置 1，开始扇区擦除
    FLASH->CR |= FLASH_CR_STRT;
    //（5）等待 FLASH_SR 寄存器中的 BSY 位清零，等待擦除操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    FLASH->CR &= ~FLASH_CR_PER;
    return 0;  //成功返回
}
//======================================================================
//函数名称：flash_write
//函数返回：函数执行状态：0=正常；1=异常。
//参数说明：sect：扇区号（范围取决于实际芯片，例如 STM32L433:0~127,每扇区2KB）
//        offset:写入扇区内部偏移地址（0~2044，要求为0,4,8,12，......）
//        N：写入字节数目（4~2048,要求为4,8,12,......）
//        buf：源数据缓冲区首地址
//功能概要：将buf开始的N字节写入到flash存储器的sect扇区的 offset处
//编程参考：暂无
//=======================================================================
uint8_t flash_write(uint16_t sect,uint16_t offset,uint16_t N,uint8_t *buf)
{
    //（1）定义变量
    uint16_t i;
    //（2）清除之前的编程导致的所有错误标志位
    FLASH->SR &= 0xFFFFFFUL;
    //（3.1）写入字节数后会跨扇区
    if(offset+N>MCU_SECTORSIZE)
    {
        //（3.1.1）先写入第一个扇区
        flash_write(sect,offset,MCU_SECTORSIZE-offset,buf);
        //（3.1.2）再写入第二个扇区
        flash_write(sect+1,0,N-(MCU_SECTORSIZE-offset),buf+(MCU_SECTORSIZE-offset));
    }
    //（3.2）写入字节数不会跨扇区
    else
    {
            uint8_t data[MCU_SECTORSIZE]; //存储当前扇区的全部值
            flash_read_logic(data,sect,0,MCU_SECTORSIZE); //将当前扇区的全部值读入数组中
            //将要写入的数据依照对应位置写入数组中
            for(i = 0;i<N;i++)
            {
                data[offset+i] = buf[i];
            }
            //擦除扇区
            flash_erase(sect);
            //将数组写入扇区
            flash_Best(sect,0,MCU_SECTORSIZE,data);
    }
    //（4）等待写入操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    return 0;  //成功执行
}

//==========================================================================
//函数名称：flash_write_physical
//函数返回：函数执行状态：0=正常；非0=异常。
//参数说明： addr：目标地址，要求为4的倍数且大于Flash首地址
//              （例如：0x08000004，Flash首地址为0x08000000）
//       cnt：写入字节数目（8~512）
//       buf：源数据缓冲区首地址
//功能概要：flash写入操作
//编程参考：暂无
//==========================================================================
uint8_t flash_write_physical(uint32_t addr,uint16_t N,uint8_t buf[])
{
    //（1）定义变量。sect-扇区号，offset-扇区地址
    uint16_t sect;   //扇区号
    uint16_t offset;    // 偏移地址
    //（2）变量赋值，将物理地址转换为逻辑地址（扇区和偏移量）
    sect = (addr-MCU_FLASH_ADDR_START)/MCU_SECTORSIZE;
    offset = addr-(sect*MCU_SECTORSIZE)-MCU_FLASH_ADDR_START;
    //（3）调用写入函数写入数据
    flash_write(sect,offset,N,buf);
    //（4）等待写入操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    return 0;  //成功执行
}

//======================================================================
//函数名称：flash_read_logic
//函数返回：无
//参数说明：dest：读出数据存放处（传地址，目的是带出所读数据，RAM区）
//       sect：扇区号（范围取决于实际芯片，例如 STM32L433:0~127,每扇区2KB）
//       offset:扇区内部偏移地址（0~2024，要求为0,4,8,12，......）
//       N：读字节数目（4~2048,要求为4,8,12,......）//
//功能概要：读取flash存储器的sect扇区的 offset处开始的N字节，到RAM区dest处
//编程参考：暂无
//=======================================================================
void flash_read_logic(uint8_t *dest,uint16_t sect,uint16_t offset,uint16_t N)
{
    //（1）定义变量。src-读取的数据的地址
    uint8_t *src;
    //（2）变量赋值。通过扇区号和偏移量计算出逻辑地址
    src=(uint8_t *)(FLASH_BASE+sect*FLASH_PAGE_SIZE+offset);
    //（3）读出数据
    memcpy(dest,src,N);
}

//======================================================================
//函数名称：flash_read_physical
//函数返回：无
//参数说明：dest：读出数据存放处（传地址，目的是带出所读数据，RAM区）
//       addr：目标地址，要求为4的倍数（例如：0x00000004）
//       N：读字节数目（0~1020,要求为4，8,12,......）
//功能概要：读取flash指定地址的内容
//编程参考：暂无
//======================================================================
void flash_read_physical(uint8_t *dest,uint32_t addr,uint16_t N)
{
    //（1）定义变量。src-读取的数据的地址
    uint8_t *src;
    //（2）变量赋值。直接使用物理地址
    src=(uint8_t *)addr;
    //（3）读出数据
    memcpy(dest,src,N);
}

//======================================================================
//函数名称：flash_protect
//函数返回：无
//参数说明：M：待保护的扇区号
//功能概要：flash保护操作
//编程参考：暂无
//======================================================================
void flash_protect(uint8_t M)
{
    //（1）定义变量。temp-配置所用值
    uint32_t temp;
    //（2）变量赋值。
    temp = M|((M+1)<<16);
    //（3）对保护区进行锁定
    if((FLASH->CR & FLASH_CR_OPTLOCK) != 0U)
    {
        FLASH->OPTKEYR = (uint32_t)FLASH_OPTKEY1;
        FLASH->OPTKEYR = (uint32_t)FLASH_OPTKEY2;
    }
    //（4）等待锁定成功
    while((FLASH->CR & FLASH_CR_OPTLOCK) != 0U);
    //（5）等待 FLASH_SR 寄存器中的 BSY 位清零，等待擦除操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    //（6）配置相关寄存器
    FLASH->WRP1AR &= 0x0;
    FLASH->WRP1AR |= temp;
    FLASH->CR |= FLASH_CR_OPTSTRT;
    //（7）等待 FLASH_SR 寄存器中的 BSY 位清零，等待擦除操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    FLASH->CR &= ~FLASH_CR_OPTSTRT;

}


//======================================================================
//函数名称：flash_unprotect
//函数返回：无
//参数说明：M：待解保护的扇区号
//功能概要：flash解保护操作
//说明：
//编程参考：暂无
//======================================================================
void flash_unprotect(uint8_t M)
{
    //（1）定义变量。temp-配置所用值
    uint32_t temp;
    //（2）变量赋值。
    temp = 0xff00ffff;
    //（3）对保护区进行锁定
    if((FLASH->CR & FLASH_CR_OPTLOCK) != 0U)
    {
        FLASH->OPTKEYR = (uint32_t)FLASH_OPTKEY1;
        FLASH->OPTKEYR = (uint32_t)FLASH_OPTKEY2;
    }
    //（4）等待 FLASH_SR 寄存器中的 BSY 位清零，等待操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    //（5）配置相关寄存器
    FLASH->WRP1AR &= 0x0;
    FLASH->WRP1AR |= temp;
    FLASH->CR |= FLASH_CR_OPTSTRT;
    //（6）等待 FLASH_SR 寄存器中的 BSY 位清零，等待操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    FLASH->CR &= ~FLASH_CR_OPTSTRT;
}

//======================================================================
//函数名称：flash_isempty
//函数返回：1=目标区域为空；0=目标区域非空。
//参数说明：所要探测的flash区域初始地址
//功能概要：flash判空操作
//编程来源：暂无
//======================================================================

uint8_t flash_isempty(uint16_t sect,uint16_t N)
{
    //（1）定义变量flag-操作成功标志，src-目标地址，dest[]-暂存数据
    uint16_t i,flag;
    uint8_t dest[N];
    uint8_t *src;
    //（2）变量赋值并读取数据
    flag = 1;
    src=(uint8_t *)(FLASH_BASE+sect*FLASH_PAGE_SIZE);
    memcpy(dest,src,N);
    //（3）判断区域内数据是否为空
    for(i = 0; i<N; i++)   //遍历区域内字节
    {
        if(dest[i]!=0xff)   //非空
        {
            flag=0;
            break;
        }
    }
    return flag;
}
//========================================================================
//函数名称：flashCtl_isSectorProtected
//函数返回：1=扇区被保护；0=扇区未被保护
//参数说明：所要检测的扇区
//功能概要：判断flash扇区是否被保护
//编程参考：暂无
//=========================================================================
uint8_t flash_isSectorProtected(uint16_t sect)
{
    //（1）定义变量。STRT-扇区开始号，END-扇区结束号，temp-暂存值
    uint8_t STRT,END;
    uint32_t temp;
    //（2）查看扇区是否被保护
    temp = FLASH->WRP1AR;
    STRT = temp;
    END = temp>>16;
    if(sect>=STRT && sect<END)
    return 1;
    //（3）查看扇区是否被保护
    temp = FLASH->WRP1BR;
    STRT = temp;
    END = temp>>16;
    if(sect>=STRT && sect<END)
    return 1;
    return 0;
}

//----------------------以下为内部函数存放处----------------------------
//======================================================================
//函数名称：flash_write_DoubleWord
//函数返回：0-成功 1-失败
//参数说明：addr：目标地址，要求为4的倍数且大于Flash首地址
//              （例如：0x08000004，Flash首地址为0x08000000）
//       data：写入的双字
//功能概要：Flash双字写入操作（STM32L433每次只能实现双字写入，先写低位字，再写高位字）
//编程参考：暂无
//======================================================================
uint8_t flash_write_DoubleWord(uint32_t addr,uint32_t data_l,uint32_t data_h)
{
    //（1）清数据缓冲区
    if((FLASH->ACR & FLASH_ACR_DCEN) != 0U)
    {
        FLASH->ACR &= ~FLASH_ACR_DCEN;
    }
    //（2）使能Flash即时编程
    FLASH->CR |= FLASH_CR_PG;
    //（3.1）先写入低位字
    *(__IO uint32_t*)addr = data_l;
    //（3.2）再写入高位字
    *(__IO uint32_t*)(addr+4U) = data_h;
    //（4）禁止Flash即时编程
    FLASH->CR &= ~FLASH_CR_PG;
    return 0;    //返回成功
}
#endif


//======================================================================
//函数名称：flash_Best
//函数返回：0-成功 1-失败
//参数说明：sect:待写入扇区号
//            offset:待写入数据位置的偏移地址
//            N：待写入数据字节数
//            buf:待写入数据的首地址
//功能概要：首位地址都对齐的情况下的数据写入
//编程参考：暂无
//======================================================================
uint8_t flash_Best(uint16_t sect,uint16_t offset,uint16_t N,uint8_t *buf)
{
    //（1）定义变量。temp_l,temp_h,addr,i
    uint32_t temp_l,temp_h;
    uint32_t addr;
    uint16_t i;
    //（2）等待 FLASH_SR 寄存器中的 BSY 位清零，等待操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    //（3）计算双字写入绝对地址
    addr = (uint32_t)(FLASH_BASE+sect*FLASH_PAGE_SIZE+offset);
    //（4）循环写入双字，每8个数写一次
    for(i = 0; i < N; i+=8)
    {
        //（4.1）低位字赋值
        temp_l = (uint32_t)((buf[i])|(buf[i+1]<<8)|(buf[i+2]<<16)|(buf[i+3]<<24));
        //（4.2）高位字赋值
        temp_h = (uint32_t)((buf[i+4])|(buf[i+5]<<8)|(buf[i+6]<<16)|(buf[i+7]<<24));
        //（4.3）在绝对地址(addr+i)，实现双字写入
        flash_write_DoubleWord((addr+i),temp_l,temp_h);
    }
    //（5）等待 FLASH_SR 寄存器中的 BSY 位清零，等待操作完成
    while( (FLASH->SR & FLASH_SR_BSY) != 0U);
    return 0;
}

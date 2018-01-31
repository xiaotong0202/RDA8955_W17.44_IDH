/* Copyright (C) 2016 RDA Technologies Limited and/or its affiliates("RDA").
* All rights reserved.
*
* This software is supplied "AS IS" without any warranties.
* RDA assumes no responsibility or liability for the use of the software,
* conveys no license or title under any patent, copyright, or mask work
* right to the product. RDA reserves the right to make changes in the
* software without notification.  RDA also make no representation or
* warranty that such application will be suitable for the specified use
* without further testing or modification.
*/







struct Map9_Latch
{
    BYTE lo_bank;
    BYTE hi_bank;
    BYTE state;
};

struct Map9_Latch latch1;
struct Map9_Latch latch2;




void Map9_Init()
{
    int nPage;


    MapperInit = Map9_Init;


    MapperWrite = Map9_Write;


    MapperSram = Map0_Sram;


    MapperApu = Map0_Apu;


    MapperReadApu = Map0_ReadApu;


    MapperVSync = Map0_VSync;


    MapperHSync = Map0_HSync;


    MapperPPU = Map9_PPU;


    MapperRenderScreen = Map0_RenderScreen;


    SRAMBANK = SRAM;


    ROMBANK0 = ROMPAGE( 0 );
    ROMBANK1 = ROMLASTPAGE( 2 );
    ROMBANK2 = ROMLASTPAGE( 1 );
    ROMBANK3 = ROMLASTPAGE( 0 );


    if ( NesHeader.byVRomSize > 0 )
    {
        for ( nPage = 0; nPage < 8; ++nPage )
            PPUBANK[ nPage ] = VROMPAGE( nPage );
        InfoNES_SetupChr();
    }


    latch1.state = 0xfe;
    latch1.lo_bank = 0;
    latch1.hi_bank = 0;
    latch2.state = 0xfe;
    latch2.lo_bank = 0;
    latch2.hi_bank = 0;


    K6502_Set_Int_Wiring( 1, 1 );
}




void Map9_Write( WORD wAddr, BYTE byData )
{
    WORD wMapAddr;

    wMapAddr = wAddr & 0xf000;
    switch ( wMapAddr )
    {
        case 0xa000:

            byData %= ( NesHeader.byRomSize << 1 );
            ROMBANK0 = ROMPAGE( byData );
            break;

        case 0xb000:

            byData %= ( NesHeader.byVRomSize << 1 );
            byData <<= 2;


            latch1.lo_bank = byData;

            if (0xfd == latch1.state)
            {

                PPUBANK[ 0 ] = VROMPAGE( byData );
                PPUBANK[ 1 ] = VROMPAGE( byData + 1 );
                PPUBANK[ 2 ] = VROMPAGE( byData + 2 );
                PPUBANK[ 3 ] = VROMPAGE( byData + 3 );
                InfoNES_SetupChr();
            }
            break;

        case 0xc000:

            byData %= ( NesHeader.byVRomSize << 1 );
            byData <<= 2;


            latch1.hi_bank = byData;

            if (0xfe == latch1.state)
            {

                PPUBANK[ 0 ] = VROMPAGE( byData );
                PPUBANK[ 1 ] = VROMPAGE( byData + 1 );
                PPUBANK[ 2 ] = VROMPAGE( byData + 2 );
                PPUBANK[ 3 ] = VROMPAGE( byData + 3 );
                InfoNES_SetupChr();
            }
            break;

        case 0xd000:

            byData %= ( NesHeader.byVRomSize << 1 );
            byData <<= 2;


            latch2.lo_bank = byData;

            if (0xfd == latch2.state)
            {

                PPUBANK[ 4 ] = VROMPAGE( byData );
                PPUBANK[ 5 ] = VROMPAGE( byData + 1 );
                PPUBANK[ 6 ] = VROMPAGE( byData + 2 );
                PPUBANK[ 7 ] = VROMPAGE( byData + 3 );
                InfoNES_SetupChr();
            }
            break;

        case 0xe000:

            byData %= ( NesHeader.byVRomSize << 1 );
            byData <<= 2;


            latch2.hi_bank = byData;

            if (0xfe == latch2.state)
            {

                PPUBANK[ 4 ] = VROMPAGE( byData );
                PPUBANK[ 5 ] = VROMPAGE( byData + 1 );
                PPUBANK[ 6 ] = VROMPAGE( byData + 2 );
                PPUBANK[ 7 ] = VROMPAGE( byData + 3 );
                InfoNES_SetupChr();
            }
            break;

        case 0xf000:

            InfoNES_Mirroring( byData & 0x01 ? 0 : 1);
            break;
    }
}




void Map9_PPU( WORD wAddr )
{

    switch ( wAddr & 0x3ff0 )
    {
        case 0x0fd0:

            latch1.state = 0xfd;

            PPUBANK[ 0 ] = VROMPAGE( latch1.lo_bank );
            PPUBANK[ 1 ] = VROMPAGE( latch1.lo_bank + 1 );
            PPUBANK[ 2 ] = VROMPAGE( latch1.lo_bank + 2 );
            PPUBANK[ 3 ] = VROMPAGE( latch1.lo_bank + 3 );
            InfoNES_SetupChr();
            break;

        case 0x0fe0:

            latch1.state = 0xfe;

            PPUBANK[ 0 ] = VROMPAGE( latch1.hi_bank );
            PPUBANK[ 1 ] = VROMPAGE( latch1.hi_bank + 1 );
            PPUBANK[ 2 ] = VROMPAGE( latch1.hi_bank + 2 );
            PPUBANK[ 3 ] = VROMPAGE( latch1.hi_bank + 3 );
            InfoNES_SetupChr();
            break;

        case 0x1fd0:

            latch2.state = 0xfd;

            PPUBANK[ 4 ] = VROMPAGE( latch2.lo_bank );
            PPUBANK[ 5 ] = VROMPAGE( latch2.lo_bank + 1 );
            PPUBANK[ 6 ] = VROMPAGE( latch2.lo_bank + 2 );
            PPUBANK[ 7 ] = VROMPAGE( latch2.lo_bank + 3 );
            InfoNES_SetupChr();
            break;

        case 0x1fe0:

            latch2.state = 0xfe;

            PPUBANK[ 4 ] = VROMPAGE( latch2.hi_bank );
            PPUBANK[ 5 ] = VROMPAGE( latch2.hi_bank + 1 );
            PPUBANK[ 6 ] = VROMPAGE( latch2.hi_bank + 2 );
            PPUBANK[ 7 ] = VROMPAGE( latch2.hi_bank + 3 );
            InfoNES_SetupChr();
            break;
    }
}
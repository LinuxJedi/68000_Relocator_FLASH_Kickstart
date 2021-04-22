`timescale 1ns / 1ps
/*
    This file is part of FLASH_KICKSTART originally designed by
    Paul Raspa 2017-2018.

    FLASH_KICKSTART is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    FLASH_KICKSTART is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FLASH_KICKSTART. If not, see <http://www.gnu.org/licenses/>.

    Revision 0.0 - 08.09.2018:
    Initial revision.

    Revision 1.0 - 12.06.2019:
    Thanks to https://github.com/niklasekstrom for refactoring and addressing
    the Auto Config (R) glitch when used with a cascaded chain.

    Revision 2.0 - 22.08.2020:
    Thanks to https://github.com/niklasekstrom for again refactoring and addressing
    the issue identified by "The Q!".

    Revision 2.1 - 26.12.2020:
    Switch power-on default to boot from motherboard ROM.

    Revision 3.0 - 2020-12-28:
    * Hook up A19 and allow for a multi-rom switch if 1MB is installed

    Revision 3.1 - 2021-03-03:
    * Tidy up firmware a bit

    Revision 3.2 - 2021-03-10:
    * Fix version number serial (there are special releases)
    * Allow board to work outside the 8MB area

    Revision 3.3 - 2021-04-06:
    * Fix issue which causes boot loop with ROM2 on some Amigas

    Revision 3.4 - 2021-04-22:
    * Jumper now selects which ROM to boot from first (motherboard or ROM1),
      no more 512K / 1 ROM support.
    * Fix issue where held reset on first boot switches ROM (PiStorm boot)
*/

 module FLASH_KICKSTART(
    input CLK, //CLK is unused
    input E_CLK,

    input RESET_n,
    input CPU_AS_n,
    input LDS_n,
    input UDS_n,
    input RW,

    output MB_AS_n,
    output DTACK_n,

    input [23:16] ADDRESS_HIGH,
    input [7:1] ADDRESS_LOW,
    inout [15:12] DATA,

    output [1:0] FLASH_WR_n,
    output [1:0] FLASH_RD_n,
    output FLASH_A19,

    input FIRST_ROM
);

// Autoconfig params
localparam [15:0] manufacturer_id = 16'h07B9;
localparam [7:0] product_id = 8'd104;
localparam [31:0] serial = 32'h00003040; // Version number

// Set by the FIRST_ROM jumper
reg useMotherboardKickstart = 1'b1;

reg [19:0] switchCounter = 20'd0;
reg hasSwitched = 1'b0;

reg autoConfigComplete = 1'b0;
reg [3:0] flashBase = 4'h0;
reg flashBaseValid = 1'b0;

reg overlay_n = 1'b0;

reg [3:0] dataOut = 4'h0;

// Set by the FIRST_ROM jumper
reg useLowRom = 1'b0;

// Some things (such as PiStorm) can hold the reset down on first boot until
// the CPU is ready, if this happens for too long this will cause the ROM
// to switch, so hold off switching until we have had one CIA access since
// power on.
reg firstBoot = 1'b0;

wire ciaRange               = ADDRESS_HIGH[23:16] == 8'hBF;
wire autoConfigRange        = ADDRESS_HIGH[23:16] == 8'hE8;
wire kickstartRange         = ADDRESS_HIGH[23:19] == 5'h1F; // f80000 - ff0000
wire kickstartOverlayRange  = ADDRESS_HIGH[23:16] == 8'h00;
wire flashRange             = ADDRESS_HIGH[23:20] == flashBase && flashBaseValid;

wire relocatorKickstartAccess   = !useMotherboardKickstart && (kickstartRange || (!overlay_n && kickstartOverlayRange));
wire autoConfigAccess           = useMotherboardKickstart && autoConfigRange && !autoConfigComplete;
wire flashAccess                = useMotherboardKickstart && flashRange;

wire relocatorAccess            = relocatorKickstartAccess || autoConfigAccess || flashAccess;

// A19 needs to be address 19 with motherboard ROM
// Otherwise the inverse of useLowRom
assign FLASH_A19 = useMotherboardKickstart ? ADDRESS_HIGH[19] : !useLowRom;

always @(posedge E_CLK or posedge RESET_n)
begin
    if (RESET_n)
    begin
        switchCounter <= 20'd0;
        hasSwitched <= 1'b0;
    end
    else if (firstBoot)
    begin
        switchCounter <= switchCounter + 20'd1;
        if (!hasSwitched && (&switchCounter))
        begin
            hasSwitched <= 1'b1;
            useMotherboardKickstart <= !useLowRom && !useMotherboardKickstart;
            useLowRom <= !useLowRom && useMotherboardKickstart;
        end
    end
    else
    begin
        useMotherboardKickstart <= FIRST_ROM;
        useLowRom <= !FIRST_ROM;
    end
end

always @(posedge CPU_AS_n or negedge RESET_n)
begin
    if (!RESET_n)
        overlay_n <= 1'b0;
    else if (ciaRange)
    begin
        firstBoot <= 1'b1;
        overlay_n <= 1'b1;
    end
end

always @(posedge CPU_AS_n or negedge RESET_n)
begin
    if (!RESET_n)
    begin
        flashBase <= 4'h0;
        flashBaseValid <= 1'b0;
        autoConfigComplete <= 1'b0;
    end
    else if (autoConfigAccess && !RW)
    begin
        if (ADDRESS_LOW == 7'h24)
        begin
            flashBase <= DATA;
            flashBaseValid <= 1'b1;
            autoConfigComplete <= 1'b1;
        end
        else if (ADDRESS_LOW == 7'h26)
            autoConfigComplete <= 1'b1;
    end
end

assign DTACK_n = !CPU_AS_n && relocatorAccess ? 1'b0 : 1'bZ;
assign MB_AS_n = !(!CPU_AS_n && !relocatorAccess);

assign FLASH_RD_n = !CPU_AS_n && (relocatorKickstartAccess || flashAccess) && RW ? {UDS_n, LDS_n} : 2'b11;
assign FLASH_WR_n = !CPU_AS_n && flashAccess && !RW ? {UDS_n, LDS_n} : 2'b11;

assign DATA = !CPU_AS_n && autoConfigAccess && RW ? dataOut : 4'bZZZZ;

always @(negedge CPU_AS_n)
begin
    if (ADDRESS_LOW[7:6] == 2'd0)
        case (ADDRESS_LOW[5:1])
            5'h00: dataOut <= 4'b1100; // Zorro II, non-RAM board
            5'h01: dataOut <= 4'b0101; // 1M
            // Inverted bits from here
            5'h02: dataOut <= ~product_id[7:4];
            5'h03: dataOut <= ~product_id[3:0];
            5'h04: dataOut <= ~4'b0000; // Any memory area. Board cannot be shut-up
            // 5 - 7 reserved, set to inverted zero using fall-through
            5'h08: dataOut <= ~manufacturer_id[15:12];
            5'h09: dataOut <= ~manufacturer_id[11:8];
            5'h0A: dataOut <= ~manufacturer_id[7:4];
            5'h0B: dataOut <= ~manufacturer_id[3:0];
            5'h0C: dataOut <= ~serial[31:28];
            5'h0D: dataOut <= ~serial[27:24];
            5'h0E: dataOut <= ~serial[23:20];
            5'h0F: dataOut <= ~serial[19:16];
            5'h10: dataOut <= ~serial[15:12];
            5'h11: dataOut <= ~serial[11:8];
            5'h12: dataOut <= ~serial[7:4];
            5'h13: dataOut <= ~serial[3:0];
            default: dataOut <= 4'hF;
        endcase
    else
        dataOut <= 4'hF;
end
endmodule

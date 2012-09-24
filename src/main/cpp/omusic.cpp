#include "omusic.hpp"

OMusic omusic;

OMusic::OMusic(void)
{
}


OMusic::~OMusic(void)
{
}

// Initialize Music Selection Screen
//
// Source: 0xB342
void OMusic::enable()
{
    oferrari.car_ctrl_active = false;
    video.clear_text_ram();
    osprites.disable_sprites();
    otraffic.disable_traffic();
    //edit jump table 3
    oinitengine.car_increment = 0;
    oferrari.car_inc_old = 0;
    osprites.spr_cnt_main = 0;
    osprites.spr_cnt_shadow = 0;
    oroad.road_ctrl = ORoad::ROAD_BOTH_P0;
    oroad.horizon_base = -0x3FF;
    blit_music_select();
    ohud.blit_text2(TEXT2_SELECT_MUSIC); // Select Music By Steering
    ostats.time_counter = 0x30; // Move 30 seconds to timer countdown (note on the original roms this is 15 seconds)
    ostats.frame_counter = ostats.frame_reset;  
    // todo: play sounds, waves etc.

    entry_start = OSprites::SPRITE_ENTRIES - 0x10;
    // Enable block of sprites
    for (int i = entry_start; i < entry_start + 5; i++)
    {
        osprites.jump_table[i].init(i);
    }

    setup_sprite1();
    setup_sprite2();
    setup_sprite3();
    setup_sprite4();
    setup_sprite5();
}

void OMusic::disable()
{
    // Enable block of sprites
    for (int i = entry_start; i < entry_start + 5; i++)
    {
        osprites.jump_table[i].control &= ~OSprites::ENABLE;
    }
}

// Music Selection Screen: Setup Radio Sprite
// Source: 0xCAF0
void OMusic::setup_sprite1()
{
    oentry *e = &osprites.jump_table[entry_start + 0];
	e->x = 28;
	e->y = 180;
	e->road_priority = 0xFF;
	e->priority = 0x1FE;
	e->zoom = 0x7F;
	e->pal_src = 0xB0;
	e->addr = SPRITE_RADIO;
	osprites.map_palette(e);
}

// Music Selection Screen: Setup Equalizer Sprite
// Source: 0xCB2A
void OMusic::setup_sprite2()
{
    oentry *e = &osprites.jump_table[entry_start + 1];
	e->x = 4;
	e->y = 189;
	e->road_priority = 0xFF;
	e->priority = 0x1FE;
	e->zoom = 0x7F;
	e->pal_src = 0xA7;
	e->addr = SPRITE_EQ;
	osprites.map_palette(e);
}

// Music Selection Screen: Setup FM Radio Readout
// Source: 0xCB64
void OMusic::setup_sprite3()
{
    oentry *e = &osprites.jump_table[entry_start + 2];
	e->x = -8;
	e->y = 176;
	e->road_priority = 0xFF;
	e->priority = 0x1FE;
	e->zoom = 0x7F;
	e->pal_src = 0x87;
	e->addr = SPRITE_FM_LEFT;
	osprites.map_palette(e);
}

// Music Selection Screen: Setup FM Radio Dial
// Source: 0xCB9E
void OMusic::setup_sprite4()
{
    oentry *e = &osprites.jump_table[entry_start + 3];
	e->x = 68;
	e->y = 181;
	e->road_priority = 0xFF;
	e->priority = 0x1FE;
	e->zoom = 0x7F;
	e->pal_src = 0x89;
	e->addr = SPRITE_DIAL_LEFT;
	osprites.map_palette(e);
}

// Music Selection Screen: Setup Hand Sprite
// Source: 0xCBD8
void OMusic::setup_sprite5()
{
    oentry *e = &osprites.jump_table[entry_start + 4];
	e->x = 21;
	e->y = 196;
	e->road_priority = 0xFF;
	e->priority = 0x1FE;
	e->zoom = 0x7F;
	e->pal_src = 0xAF;
	e->addr = SPRITE_HAND_LEFT;
	osprites.map_palette(e);
}

// Check for start button during music selection screen
//
// Source: 0xB768
void OMusic::check_start()
{
    if (ostats.credits && input.has_pressed(Input::START))
    {
        outrun.game_state = GS_INIT_GAME;
        ologo.disable();
        disable();
    }
}

// Tick and Blit
void OMusic::tick()
{
    // Note tiles to append to left side of text
    const uint32_t NOTE_TILES1 = 0x8A7A8A7B;
    const uint32_t NOTE_TILES2 = 0x8A7C8A7D;

    // Radio Sprite
    osprites.do_spr_order_shadows(&osprites.jump_table[entry_start + 0]);

    // Animated EQ Sprite (Cycle the graphical equalizer on the radio)
    oentry *e = &osprites.jump_table[entry_start + 1];
    e->reload++; // Increment palette entry
    e->pal_src = roms.rom0.read8((e->reload & 0x3E) >> 1 | MUSIC_EQ_PAL);
    osprites.map_palette(e);
    osprites.do_spr_order_shadows(e);

    // Draw appropriate FM station on radio, depending on steering setting
    // Draw Dial on radio, depending on steering setting
    e = &osprites.jump_table[entry_start + 2];
    oentry *e2 = &osprites.jump_table[entry_start + 3];
    oentry *hand = &osprites.jump_table[entry_start + 4];

    // Steer Left
    if (oinputs.steering_adjust + 0x80 <= 0x55)
    {
        ohud.blit_text2(TEXT2_MAGICAL);
        video.write_text32(0x1105C0, NOTE_TILES1);
        video.write_text32(0x110640, NOTE_TILES2);
        
        hand->x = 17;

        e->addr    = SPRITE_FM_LEFT;
        e2->addr   = SPRITE_DIAL_LEFT;
        hand->addr = SPRITE_HAND_LEFT;

        // move.w  #0,(music_selected).l
    }
    // Centre
    else if (oinputs.steering_adjust + 0x80 <= 0xAA)
    {
        ohud.blit_text2(TEXT2_BREEZE);
        video.write_text32(0x1105C6, NOTE_TILES1);
        video.write_text32(0x110646, NOTE_TILES2);

        hand->x = 21;

        e->addr    = SPRITE_FM_CENTRE;
        e2->addr   = SPRITE_DIAL_CENTRE;
        hand->addr = SPRITE_HAND_CENTRE;

        // move.w  #1,(music_selected).l
    }
    // Steer Right
    else
    {
        ohud.blit_text2(TEXT2_SPLASH);
        video.write_text32(0x1105C8, NOTE_TILES1);
        video.write_text32(0x110648, NOTE_TILES2);

        hand->x = 21;

        e->addr    = SPRITE_FM_RIGHT;
        e2->addr   = SPRITE_DIAL_RIGHT;
        hand->addr = SPRITE_HAND_RIGHT;

        // move.w  #2,(music_selected).l
    }
    osprites.do_spr_order_shadows(e);
    osprites.do_spr_order_shadows(e2);
    osprites.do_spr_order_shadows(hand);
}

// Blit Only: Used when frame skipping
void OMusic::blit()
{
    for (int i = 0; i < 5; i++)
        osprites.do_spr_order_shadows(&osprites.jump_table[entry_start + i]);
}

// Blit Music Selection Tiles to text ram layer (Double Row)
// 
// Source Address: 0xE0DC
// Input:          Destination address into tile ram
// Output:         None
//
// Tilemap data is stored in the ROM as a series of words.
//
// A basic compression format is used:
//
// 1/ If a word is not '0000', copy it directly to tileram
// 2/ If a word is '0000' a long follows which details the compression.
//    The upper word of the long is the value to copy.
//    The lower word of the long is the number of times to copy that value.
//
// Tile structure:
//
// MSB          LSB
// ---nnnnnnnnnnnnn Tile index (0-8191)
// ---ccccccc------ Palette (0-127)
// p--------------- Priority flag
// -??------------- Unknown


void OMusic::blit_music_select()
{
    const uint32_t TILEMAP_RAM_16 = 0x10F030;

    // Palette Ram: 1F Long Entries For Sky Shade On Horizon, For Colour Change Effect
    const uint32_t PAL_RAM_SKY = 0x120F00;

	uint32_t src_addr = PAL_MUSIC_SELECT;
	uint32_t dst_addr = PAL_RAM_SKY;

	// Write 32 Palette Longs to Palette RAM
	for (int i = 0; i < 32; i++)
	{
		video.write_pal32(&dst_addr, roms.rom0.read32(&src_addr));
	}

    otiles.reset_scroll();

	src_addr = TILEMAP_MUSIC_SELECT;

	// Blit to tilemap 16
	uint32_t tilemap16 = TILEMAP_RAM_16;
	
	for (int y = 0; y <= 27; y++)
	{
		dst_addr = tilemap16;
		for (int x = 0; x <= 39;)
		{
			// get next tile
			uint32_t data = roms.rom0.read16(&src_addr);
			// No Compression: write tile directly to tile ram
			if (data != 0)
			{
				video.write_tile16(&dst_addr, data);		
				x++;
			}
			// Compression
			else
			{
				uint16_t value = roms.rom0.read16(&src_addr); // tile index to copy
				uint16_t count = roms.rom0.read16(&src_addr); // number of times to copy value

				for (uint16_t i = 0; i <= count; i++)
				{
					video.write_tile16(&dst_addr, value);
					x++;
				}
			}
		}
		tilemap16 += 0x80; // next line of tiles
	} // end for

    // Fix Misplaced tile on music select screen (above steering wheel)
    if (FIX_BUGS)
    {
        video.write_tile16(0x10F730, 0x0C80);
    }
}
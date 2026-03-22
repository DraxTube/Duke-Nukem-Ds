/*---------------------------------------------------------------------------------

	default ARM7 core - updated for modern libnds (2024+)

		Copyright (C) 2005 - 2010
		Michael Noland (joat), Jason Rogers (dovoto), Dave Murphy (WinterMute)

	Changes vs original:
	- irqInit()            removed (implicit in modern libnds)
	- inputGetAndSend()    replaced by updateInputRegX()
	- readUserSettings()   replaced by readUserSettingsFromFirmware()
	- initClockIRQ()       removed (handled internally by libnds)
	- fifoInit()           removed (automatic)
	- installSoundFIFO()   replaced by soundEmptyFIFO()
	- installSystemFIFO()  replaced by systemMsgHandler()
	- setPowerButtonCB()   replaced by powerButtonSetCallback()

---------------------------------------------------------------------------------*/
#include <nds.h>

void VblankHandler(void) {
}

void VcountHandler() {
	updateInputRegX();
}

volatile bool exitflag = false;

void powerButtonCB() {
	exitflag = true;
}

int main() {
	// Clear sound registers
	dmaFillWords(0, (void*)0x04000400, 0x100);

	REG_SOUNDCNT |= SOUND_ENABLE;
	REG_SNDCAP0CNT = 0;
	REG_SNDCAP1CNT = 0;
	writePowerManagement(PM_CONTROL_REG,
		(readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE) | PM_SOUND_AMP);
	powerOn(POWER_SOUND);

	readUserSettingsFromFirmware();
	ledBlink(0);

	SetYtrigger(80);

	soundEmptyFIFO();
	systemMsgHandler();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqEnable(IRQ_VBLANK | IRQ_VCOUNT);

	powerButtonSetCallback(powerButtonCB);

	while (!exitflag) {
		if (0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			exitflag = true;
		}
		swiWaitForVBlank();
	}
	return 0;
}

#include "Headers.h"
#include "Disc_DVD_ROM.h"
#include "SCSI/scsi_GetConfiguration.h"

Disc_DVD_ROM::Disc_DVD_ROM(Drive &d) : 
    Disc(d),
    rds(dio, result)
{
    activewriteprotect = 0;
    writeprotectmethods = 0;
};

bool Disc_DVD_ROM::Init()
{
    const IOptItem *disc = GetContents();
    Feat_WriteProtect *f;

    for (int i=0; i<disc->getChildCount(); i++)
    {
	const IOptItem *sess = disc->getChild(i);
	const_cast<IOptItem*>(sess)->setSectorSize(2048);
    }
    disc->release();

    Notify(result(DRT_Operation_Analyse_ControlBlocks));
    Feat_DiscControlBlocks *feat;
    feat = drive.GetFeatures().GetFeature<Feat_DiscControlBlocks>();
    dcbs.Empty();
    if (feat != 0)
    {
	for (int i=0; i<feat->GetBodyLength(); i+=4)
	{
	    uint32 f = feat->features[i/4];
	    _D(Lvl_Info, "Media offers control block: %08lx <%lc%lc%lc>", f, (f>>24)&255, (f>>16)&255, (f>>8)&255);
	    dcbs << f;
	}
    }

    CheckWriteProtected();

    /*
     * check write protection methods
     */
    writeprotectmethods = 0;
    f = drive.GetFeatures().GetFeature<Feat_WriteProtect>();
    if (f && f->IsCurrent())
    {
	if (f->conf.doesWDCB())
	    writeprotectmethods |= WP_Via_SendDVD_0x30;
	if (f->conf.doesPWP())
	    writeprotectmethods |= WP_Via_SendDVD_0xC0;
	if (f->conf.doesSWPP())
	    writeprotectmethods |= WP_Via_ModePage_0x1D;
    }
    _D(Lvl_Info, "Supported write protection model %ld", writeprotectmethods);

    return subInit();
};

bool Disc_DVD_ROM::subInit()
{
    cb = rds.ReadDVDStructure<DVD_DiscControlBlock>(0xffffffff, 0);
    // it's not necessary, but let's have it :-)

    return true;
}

Disc_DVD_ROM::~Disc_DVD_ROM(void)
{
};

void Disc_DVD_ROM::FillDiscSpeed(DiscSpeed &spd)
{
    spd.begin_f = (((int32)10*spd.begin_kbps)/1385)%10;
    spd.begin_i = ((int32)spd.begin_kbps)/1385;
    spd.end_f = (((int32)10*spd.end_kbps)/1385)%10;
    spd.end_i = ((int32)spd.end_kbps)/1385;
}

void Disc_DVD_ROM::CheckWriteProtected()
{
    activewriteprotect = 0;

    if (writeprotectmethods & WP_Via_SendDVD_0xC0)
    {
	_D(Lvl_Info, "Reading disc writeprotection info");
	AutoPtrT<Disc_WriteProtection> ds;
	if (isDVD())
	    ds = rds.ReadDVDStructure<Disc_WriteProtection>(0, 0);
	else if (isBD())
	    ds = rds.ReadBDStructure<Disc_WriteProtection>(0, 0);


	if (ds.IsSet())
	{
	    if (ds->isMediaSpecificWriteInhibition())
		activewriteprotect |= WP_Via_Media;
	    if (ds->isCartridgeWriteProtection())
		activewriteprotect |= WP_Via_Cartridge;
	    if (ds->isPersistentWriteProtection())
		activewriteprotect |= WP_Via_SendDVD_0xC0;
	    if (ds->isSoftwareWriteProtection())
		activewriteprotect |= WP_Via_ModePage_0x1D;
	}
    }
    _D(Lvl_Info, "Medium enabled write protection: %ld", activewriteprotect);
}

bool Disc_DVD_ROM::IsWriteProtected()
{
    return activewriteprotect ? true : false;
}

void Disc_DVD_ROM::SetWriteProtected(bool state)
{
    if (writeprotectmethods & WP_Via_SendDVD_0xC0)
    {
	_D(Lvl_Info, "Attempting to change write protection status.");
	//_D(Lvl_Info, "WDCB password protected inhibition is not supported by software");
	Disc_WriteProtection dwp(state);
	if (isDVD())
	    rds.SendDVDStructure<Disc_WriteProtection>(&dwp);
	else if (isBD())
	    rds.SendBDStructure<Disc_WriteProtection>(&dwp);
    }
    else
    {
	_D(Lvl_Info, "Medium does not allow write protection");
    }
    CheckWriteProtected();
}

bool Disc_DVD_ROM::AllowsWriteProtect()
{
    return writeprotectmethods ? true : false;
}

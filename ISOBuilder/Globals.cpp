#include "Globals.h"
#include "CustomISOConfigPage.h"
#include "CustomISOPopUp.h"

Globals::Globals() :
    Loc(0, "ISOBUILDER"),
    Config("ISOBuilder")
{

    CustomISOConfigPage::InitLocale(*this);
}

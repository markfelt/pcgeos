canonRGB.rdef: generic.uih product.uih ../../PrintCom/UI/uiOptions1ASFCanonRGB.ui
canonRGBManager.obj \
canonRGBManager.eobj: printcomInclude.def geos.def heap.def geode.def \
                resource.def ec.def driver.def lmem.def file.def sem.def \
                graphics.def fontID.def font.def color.def gstring.def \
                text.def char.def win.def localize.def sllang.def \
                chunkarr.def hugearr.def timer.def system.def \
                Internal/interrup.def Internal/gstate.def \
                Internal/tmatrix.def Internal/fontDr.def \
                Internal/window.def Internal/heapInt.def \
                Internal/semInt.def sysstats.def Internal/printDr.def \
                print.def Internal/spoolInt.def Internal/serialDr.def \
                Internal/streamDr.def Internal/parallDr.def \
                Internal/videoDr.def ui.def vm.def input.def hwr.def \
                Objects/processC.def Objects/metaC.def object.def \
                geoworks.def gcnlist.def timedate.def \
                Objects/Text/tCommon.def stylesh.def iacp.def \
                Objects/uiInputC.def Objects/visC.def Objects/vCompC.def \
                Objects/vCntC.def Internal/vUtils.def Objects/genC.def \
                disk.def drive.def uDialog.def Objects/gInterC.def \
                token.def Objects/clipbrd.def Objects/gSysC.def \
                Objects/gProcC.def alb.def Objects/gFieldC.def \
                Objects/gScreenC.def Objects/gFSelC.def \
                Objects/gViewC.def Objects/gContC.def Objects/gCtrlC.def \
                Objects/gDocC.def Objects/gDocCtrl.def \
                Objects/gDocGrpC.def Objects/gEditCC.def \
                Objects/gViewCC.def Objects/gToolCC.def \
                Objects/gPageCC.def Objects/gPenICC.def \
                Objects/gGlyphC.def Objects/gTrigC.def \
                Objects/gBoolGC.def Objects/gItemGC.def \
                Objects/gDListC.def Objects/gItemC.def Objects/gBoolC.def \
                Objects/gDispC.def Objects/gDCtrlC.def Objects/gPrimC.def \
                Objects/gAppC.def Objects/gTextC.def Objects/gGadgetC.def \
                Objects/gValueC.def Objects/gToolGC.def \
                Internal/gUtils.def Objects/helpCC.def Objects/eMenuC.def \
                Objects/emomC.def Objects/emTrigC.def Internal/uProcC.def \
                spool.def Internal/prodFeatures.def rgb2cmyk.def \
                printcomConstant.def printcomMacro.def \
                canonRGBConstant.def canonRGB.rdef printcomEntry.asm \
                printcomInfo.asm printcomAdmin.asm printcomTables.asm \
                printcomNoEscapes.asm Job/jobStartCanonRGB.asm \
                Job/jobEndCanonRGB.asm Job/jobPaperPathNoASFControl.asm \
                Job/jobPaperInfo.asm Job/jobResetPrinterAndWait.asm \
                canonRGBColor.asm printcomCanonRGBPage.asm \
                Page/pageStartCanonRGB.asm Page/pageEndCanonRGB.asm \
                printcomCanonBJCCursor.asm \
                Cursor/cursorDotMatrixCommon.asm \
                Cursor/cursorSetCursorAbsCanonBJC.asm \
                Cursor/cursorPrLineFeedCanonBJC.asm \
                Cursor/cursorPrFormFeedGuess.asm \
                Cursor/cursorConvert360.asm printcomStream.asm \
                Stream/streamWrite.asm Stream/streamWriteByte.asm \
                Stream/streamSendCodeOut.asm printcomCanonBJCColor.asm \
                Color/colorGetFormat.asm Color/colorSetNone.asm \
                Graphics/graphicsCommon.asm Graphics/graphicsCanonRGB.asm \
                canonRGBDialog.asm UI/uiGetNoMain.asm UI/uiGetOptions.asm \
                UI/uiEval.asm canonRGBControlCodes.asm \
                printcomNoStyles.asm printcomNoText.asm \
                Text/textPrintRaw.asm Buffer/bufferCreateCanonRGB.asm \
                Buffer/bufferDestroy.asm canonRGBDriverInfo.asm \
                canonRGBInfo.asm canonRGBmonoInfo.asm \
                canonRGBmono2Info.asm

canonrgbEC.geo canonrgb.geo : geos.ldf ui.ldf spool.ldf rgb2cmyk.ldf 

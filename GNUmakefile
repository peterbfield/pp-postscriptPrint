# Global Makefile Inclusions
ALLMAKES = $(wildcard *makesrc)

default:
	@echo "Please specify one of the following:"
	@echo
	@echo "	addshape aupi aupicvt aupimn av2mac avxpc bpm bspellbin"
	@echo " buildct cklist ckregex"
	@echo "	dm dictionary diction dictprint	em eqdb	et export_penta	facs fam"
	@echo "	famcvt fexpr fn	fnkncvt	fnx foprint genstyletree gentag	gio hnj"
	@echo "	import issgml keyb kn layrpt lpm lpminfo lpmq lq lpmset"
	@echo "	lu lwf mac2av mactopfb manpsbin	mp mvsh	ovs_update ovscvt pack_pixel"
	@echo "	pcnfsd pcs pmenus pfbtoascii pfsd pil pixel_header pixel pp pp_to_eps"
	@echo "	prfilter psftf pspellbin rag scank script setbbox settight sfed "
	@echo "	sgmle sgmlmerge sgmlp sgmlpub sgmltaglist sl splitfile sprint"
	@echo " sysconf sysstd t1_bitmap t1shape tagtotext tcltk tpsend transbbox"
	@echo " trunc ts txcontx2 txdisp txheader txoverlap txprint untrunc  usft vpfm"
	@echo "	all clean co ctags depend etags rcsclean rcsout update"
	@echo
	@echo "or to create the makefiles for another exec:"
	@echo
	@echo "	ADDSHAPE AUPI AUPICVT AUPIMN AV2MAC AVXPC BPM BSPELLBIN"
	@echo " BUILDCT CKLIST CKREGEX"
	@echo "	DM DICTIONARY DICTION DICTPRINT	EM EQDB	ET EXPORT_PENTA	FACS FAM"
	@echo "	FAMCVT FEXPR FN	FNKNCVT	FNX FOPRINT GENSTYLETREE GENTAG	GIO HNJ"
	@echo "	IMPORT ISSGML KEYB KN LAYRPT LPM LPMINFO LPMQ LQ LPMSET"
	@echo "	LU LWF MAC2AV MACTOPFB MANPSBIN	MP MVSH	OVS_UPDATE OVSCVT PACK_PIXEL"
	@echo "	PCNFSD PCS PMENUS PFBTOASCII PFSD PIL PIXEL_HEADER PIXEL PP PP_TO_EPS"
	@echo "	PRFILTER PSFTF PSPELLBIN RAG SCANK SCRIPT SETBBOX SETTIGHT SFED "
	@echo "	SGMLE SGMLMERG ESGMLP SGMLPUB SGMLTAGLIST SL SPLITFILE SPRINT"
	@echo " SYSCONF SYSSTD T1_BITMAP T1SHAPE TAGTOTEXT TCLTK TPSEND TRANSBBOX"
	@echo "	TRUNC TS TXCONTX2 TXDISP TXHEADER TXOVERLAP TXPRINT UNTRUNC USFT VPFM"
	@echo

SUFFIXES:
SUFFIXES: .o .c .a .uil .uid

%: %.sh

addshape AddShape: ADDSHAPEmakesrc ; @$(MAKE) -f $< $@

aupi: AUPImakesrc ; @$(MAKE) -f $< $@

aupicvt: AUPICVTmakesrc ; @$(MAKE) -f $< $@

aupimn: AUPIMNmakesrc ; @$(MAKE) -f $< $@

av2mac: AV2MACmakesrc ; @$(MAKE) -f $< $@

avxpc: AVXPCmakesrc ; @$(MAKE) -f $< $@

bpm: BPMmakesrc ; @$(MAKE) -f $< $@

bspellbin bspell_bin: BSPELLBINmakesrc ; @$(MAKE) -f $< $@

buildct: BUILDCTmakesrc ; @$(MAKE) -f $< $@

cklist: CKLISTmakesrc ; @$(MAKE) -f $< $@

ckregex: CKREGEXmakesrc ; @$(MAKE) -f $< $@

dm designmaster: DMmakesrc ; @$(MAKE) -f $< $@

dictionary dictionary_update: DICTIONARYmakesrc ; @$(MAKE) -f $< $@

diction diction_update: DICTIONmakesrc ; @$(MAKE) -f $< $@

dictprint: DICTPRINTmakesrc ; @$(MAKE) -f $< $@

em EditMaster: EMmakesrc ; @$(MAKE) -f $< $@

eqdb: EQDBmakesrc ; @$(MAKE) -f $< $@

et: ETmakesrc ; @$(MAKE) -f $< $@

export_penta: EXPORT_PENTAmakesrc ; @$(MAKE) -f $< $@

facs: FACSmakesrc ; @$(MAKE) -f $< $@

fam: FAMmakesrc ; @$(MAKE) -f $< $@

famcvt: FAMCVTmakesrc ; @$(MAKE) -f $< $@

fexpr: FEXPRmakesrc ; @$(MAKE) -f $< $@

fn: FNmakesrc ; @$(MAKE) -f $< $@

fnkncvt: FNKNCVTmakesrc ; @$(MAKE) -f $< $@

fnx: FNXmakesrc ; @$(MAKE) -f $< $@

foprint: FOPRINTmakesrc ; @$(MAKE) -f $< $@

genstyletree: GENSTYLETREEmakesrc ; @$(MAKE) -f $< $@

gentag gentag_update: GENTAGmakesrc ; @$(MAKE) -f $< $@

gio: GIOmakesrc ; @$(MAKE) -f $< $@

hnj HandJ: HNJmakesrc ; @$(MAKE) -f $< $@

import: IMPORTmakesrc ; @$(MAKE) -f $< $@

issgml: ISSGMLmakesrc ; @$(MAKE) -f $< $@

keyb: KEYBmakesrc ; @$(MAKE) -f $< $@

kn: KNmakesrc ; @$(MAKE) -f $< $@

layrpt: LAYRPTmakesrc ; @$(MAKE) -f $< $@

lpm: LPMmakesrc ; @$(MAKE) -f $< $@

lpminfo: LPMINFOmakesrc ; @$(MAKE) -f $< $@

lpmq: LPMQmakesrc ; @$(MAKE) -f $< $@

lq lpmquery: LQmakesrc ; @$(MAKE) -f $< $@

lpmset lpmreset: LPMSETmakesrc ; @$(MAKE) -f $< $@

lu: LUmakesrc ; @$(MAKE) -f $< $@

lwf: LWFmakesrc ; @$(MAKE) -f $< $@

mac2av: MAC2AVmakesrc ; @$(MAKE) -f $< $@

mactopfb: MACTOPFBmakesrc ; @$(MAKE) -f $< $@

manpsbin manps_bin: MANPSBINmakesrc ; @$(MAKE) -f $< $@

mp MasterPage: MPmakesrc ; @$(MAKE) -f $< $@

mvsh: MVSHmakesrc ; @$(MAKE) -f $< $@

ovs_update: OVS_UPDATEmakesrc ; @$(MAKE) -f $< $@

ovscvt: OVSCVTmakesrc ; @$(MAKE) -f $< $@

pack_pixel: PACK_PIXELmakesrc ; @$(MAKE) -f $< $@

pcnfsd: PCNFSDmakesrc ; @$(MAKE) -f $< $@

pcs PCServer: PCSmakesrc ; @$(MAKE) -f $< $@

pmenus pentamenus: PMENUSmakesrc ; @$(MAKE) -f $< $@

pfbtoascii: PFBTOASCIImakesrc ; @$(MAKE) -f $< $@

pfsd Pfsd: PFSDmakesrc ; @$(MAKE) -f $< $@

pil: PILmakesrc ; @$(MAKE) -f $< $@

pixel_header: PIXEL_HEADERmakesrc ; @$(MAKE) -f $< $@

pixel pixel_mod: PIXELmakesrc ; @$(MAKE) -f $< $@

pp: PPmakesrc ; @$(MAKE) -f $< $@

pp_to_eps: PP_TO_EPSmakesrc ; @$(MAKE) -f $< $@

prfilter: PRFILTERmakesrc ; @$(MAKE) -f $< $@

psftf: PSFTFmakesrc ; @$(MAKE) -f $< $@

pspellbin pspell_bin: PSPELLBINmakesrc ; @$(MAKE) -f $< $@

rag: RAGmakesrc ; @$(MAKE) -f $< $@

scank: SCANKmakesrc ; @$(MAKE) -f $< $@

script: SCRIPTmakesrc ; @$(MAKE) -f $< $@

setbbox SetBbox: SETBBOXmakesrc ; @$(MAKE) -f $< $@

settight SetTight: SETTIGHTmakesrc ; @$(MAKE) -f $< $@

sfed: SFEDmakesrc ; @$(MAKE) -f $< $@

sgmle: SGMLEmakesrc ; @$(MAKE) -f $< $@

sgmlmerge SgmlMerge: SGMLMERGEmakesrc ; @$(MAKE) -f $< $@

sgmlp: SGMLPmakesrc ; @$(MAKE) -f $< $@

sgmlpub: SGMLPUBmakesrc ; @$(MAKE) -f $< $@

sgmltaglist SGMLTagList: SGMLTAGLISTmakesrc ; @$(MAKE) -f $< $@

sl: SLmakesrc ; @$(MAKE) -f $< $@

splitfile: SPLITFILEmakesrc ; @$(MAKE) -f $< $@

sprint: SPRINTmakesrc ; @$(MAKE) -f $< $@

sysconf: SYSCONFmakesrc ; @$(MAKE) -f $< $@

sysstd: SYSSTDmakesrc ; @$(MAKE) -f $< $@

t1_bitmap: T1_BITMAPmakesrc ; @$(MAKE) -f $< $@

t1shape: T1SHAPEmakesrc ; @$(MAKE) -f $< $@

tagtotext: TAGTOTEXTmakesrc ; @$(MAKE) -f $< $@

tcltk: TCLTKmakesrc ; @$(MAKE) -f $< $@

tpsend: TPSENDmakesrc ; @$(MAKE) -f $< $@

transbbox TranslateBbox: TRANSBBOXmakesrc ; @$(MAKE) -f $< $@

trunc: TRUNCmakesrc ; @$(MAKE) -f $< $@

ts: TSmakesrc ; @$(MAKE) -f $< $@

txcontx2: TXCONTX2makesrc ; @$(MAKE) -f $< $@

txdisp: TXDISPmakesrc ; @$(MAKE) -f $< $@

txheader: TXHEADERmakesrc ; @$(MAKE) -f $< $@

txoverlap: TXOVERLAPmakesrc ; @$(MAKE) -f $< $@

txprint: TXPRINTmakesrc ; @$(MAKE) -f $< $@

untrunc: UNTRUNCmakesrc ; @$(MAKE) -f $< $@

usft: USFTmakesrc ; @$(MAKE) -f $< $@

vpfm: VPFMmakesrc ; @$(MAKE) -f $< $@


all depend update clean rcsclean co rcsout: $(ALLMAKES)
	@for i in $^; do echo "	>>> Making target \`$@' using $$i <<<"; $(MAKE) -f $$i $@; done

etags TAGS: $(ALLMAKES)
	@echo "Building GNU Emacsable TAGS table."
	@$(RM) TAGS
	@for i in $^; do echo "	>>> Making target \`$@' using $$i <<<"; $(MAKE) -f $$i TAGS; done

ctags tags: $(ALLMAKES)
	@echo "Building Vi TAGS table."
	@$(RM) tags
	@for i in $^; do echo "	>>> Making target \`$@' using $$i <<<"; $(MAKE) -f $$i tags; done

ADDSHAPE AUPI AUPICVT AUPIMN AV2MAC AVXPC BPM BSPELLBIN CKLIST CKREGEX DM DICTIONARY DICTION DICTPRINT EM EQDB ET EXPORT_PENTA FACS FAM FAMCVT FEXPR FN FNKNCVT FNX FOPRINT GENSTYLETREE GENTAG GIO HNJ IMPORT ISSGML KEYB KN LAYRPT LPM LPMINFO LPMQ LQ LPMSET LU LWF MAC2AV MACTOPFB MANPSBIN MP MVSH OVS_UPDATE OVSCVT PACK_PIXEL PCNFSD PCS PMENUS PFBTOASCII PFSD PIL PIXEL_HEADER PIXEL PP PP_TO_EPS PRFILTER PSFTF PSPELLBIN RAG SCANK SCRIPT SETBBOX SETTIGHT SFED SGMLE SGMLMERGE SGMLP SGMLPUB SGMLTAGLIST SL SPLITFILE SPRINT SYSCONF SYSSTD T1_BITMAP T1SHAPE TAGTOTEXT TCLTK TPSEND TRANSBBOX TRUNC TS TXCONTX2 TXDISP TXHEADER TXOVERLAP TXPRINT UNTRUNC USFT VPFM :
	@co $@makesrc; touch $@depend; gmake -f $@makesrc depend; echo "make $@ is finished"

.PHONY : all clean co ctags depend etags rcsclean rcsout update tags TAGS \
	ADDSHAPE AUPI AUPICVT AUPIMN AV2MAC AVXPC BPM BSPELLBIN \
	BUILDCT CKLIST CKREGEX \
	DM DICTIONARY DICTION DICTPRINT	EM EQDB	ET EXPORT_PENTA	FACS FAM \
	FAMCVT FEXPR FN	FNKNCVT	FNX FOPRINT GENSTYLETREE GENTAG	GIO HNJ \
	IMPORT ISSGML KEYB KN LAYRPT LPM LPMINFO LPMQ LQ LPMSET \
	LU LWF MAC2AV MACTOPFB MANPSBIN	MP MVSH	OVS_UPDATE OVSCVT PACK_PIXEL \
	PCNFSD PCS PMENUS PFBTOASCII PFSD PIL PIXEL_HEADER PIXEL PP PP_TO_EPS \
	PRFILTER PSFTF PSPELLBIN RAG SCANK SCRIPT SETBBOX SETTIGHT SFED \
	SGMLE SGMLMERGE SGMLP SGMLPUB SGMLTAGLIST SL SPLITFILE SPRINT \
	SYSCONF SYSSTD T1_BITMAP T1SHAPE TAGTOTEXT TCLTK TPSEND TRANSBBOX \
	TRUNC TS TXCONTX2 TXDISP TXHEADER TXOVERLAP TXPRINT UNTRUNC USFT VPFM \
	addshape aupi aupicvt aupimn av2mac avxpc bpm bspellbin \
	buildct cklist ckregex \
	dm dictionary diction dictprint	em eqdb	et export_penta	facs fam \
	famcvt fexpr fn	fnkncvt	fnx foprint genstyletree gentag	gio hnj \
	import issgml keyb kn layrpt lpm lpminfo lpmq lq lpmset \
	lu lwf mac2av mactopfb manpsbin	mp mvsh	ovs_update ovscvt pack_pixel \
	pcnfsd pcs pmenus pfbtoascii pfsd pil pixel_header pixel pp pp_to_eps \
	prfilter psftf pspellbin rag scank script setbbox settight sfed \
	sgmle sgmlmerge sgmlp sgmlpub sgmltaglist sl splitfile sprint \
	sysconf sysstd t1_bitmap t1shape tagtotext tcltk tpsend transbbox \
	trunc ts txcontx2 txdisp txheader txoverlap txprint untrunc usft vpfm \


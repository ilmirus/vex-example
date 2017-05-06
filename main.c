#include <stdio.h>
#include <stdlib.h>

#include <libvex.h>

static VexArchInfo vai;
static VexGuestExtents vge;
static VexTranslateArgs vta;

// Global for saving the intermediate results of translation from
// within the callback (instrument1)
static IRSB *irbb_current = NULL;
static int size_current = 0;

void error(const char* s) {
  puts(s);
  exit(1);
}

static void failure_exit(void) {
  error("failure_exit");
}

static void log_bytes(const HChar *bytes, SizeT nbytes) {
  printf("%s\n", bytes);
  // for (size_t i = 0; i < nbytes; i++) {
  //   printf("%s%x ", bytes[i] < 0x10 ? "0" : "", bytes[i]);
  // }
  printf("\n");
}

static Bool chase_into_ok(void *closureV, Addr addr) {
  return False;
}

static void *dispatch(void) {
    return NULL;
}

static UInt needs_self_check(
    void *opaque, 
    VexRegisterUpdates* pxControl, 
    const VexGuestExtents *vge) {
    return 0;
}

void init_vex() {
  VexControl vc;
  vc.iropt_verbosity                  = 0;
  vc.iropt_level                      = 0;
  vc.iropt_register_updates_default   = VexRegUpdSpAtMemAccess;
  vc.iropt_unroll_thresh              = 0;
  vc.guest_max_insns                  = 50; // By default, we translate 1 instruction at a time
  vc.guest_max_bytes                  = 2000; // By default, we translate 1 instruction at a time
  vc.guest_chase_thresh               = 0;
  vc.guest_chase_cond                 = 0;

  LibVEX_Init(&failure_exit,
            &log_bytes,
            0, // Debug level
            &vc);
  LibVEX_default_VexArchInfo(&vai);

  // FIXME: determinate endianess by specified guest arch
  vai.endness = VexEndnessLE;

  // Setup the translation args
  vta.arch_guest                  = VexArch_INVALID;  // to be assigned later
  vta.archinfo_guest              = vai;

  vta.arch_host                   = VexArchX86;    
  vta.archinfo_host               = vai;
  vta.guest_bytes                 = NULL;             // Set in translate_insns
  vta.guest_bytes_addr            = 0;                // Set in translate_insns
  vta.callback_opaque             = NULL;             // Used by chase_into_ok, but never actually called
  vta.chase_into_ok               = chase_into_ok;    // Always returns false
  vta.preamble_function           = NULL;
  vta.guest_extents               = &vge;
  
  vta.host_bytes                  = NULL;             // Buffer for storing the output binary
  vta.host_bytes_size             = 0;
  vta.host_bytes_used             = NULL;

  vta.instrument1                 = NULL;      // Callback we defined to help us save the IR
  vta.instrument2                 = NULL;
  vta.traceflags                  = 0;   // Debug verbosity
  
  vta.disp_cp_chain_me_to_slowEP  = dispatch;         // Not used
  vta.disp_cp_chain_me_to_fastEP  = dispatch;         // Not used
  vta.disp_cp_xindir              = dispatch;         // Not used
  vta.disp_cp_xassisted           = dispatch;         // Not used

  vta.needs_self_check            = needs_self_check; // Not used
}

void translate_insn() {
  char inst[] = {'\x53\x55\x56\x57'};

  vta.arch_guest = VexArchX86;
  vta.archinfo_guest.hwcaps = 0;
  vta.guest_bytes = inst; // Ptr to actual bytes of start of instruction
  vta.guest_bytes_addr = 0;

  VexTranslateResult res;
  VexRegisterUpdates pxControl;

  IRSB *irsb = LibVEX_Lift(&vta, &res, &pxControl);
  ppIRSB(irsb);
}

int main() {
  init_vex();
  translate_insn();
}

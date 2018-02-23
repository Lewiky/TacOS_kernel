#include "hilevel.h"

pcb_t pcb[ 2 ]; int executing = 0;

void scheduler( ctx_t* ctx ) {
  int nextProcess = (executing+1)%3;             // determine next process to run

  memcpy( &pcb[ executing ].ctx, ctx, sizeof( ctx_t ) ); // preserve P_1
  pcb[ executing ].status = STATUS_READY;                // update   P_1 status
  memcpy( ctx, &pcb[ nextProcess ].ctx, sizeof( ctx_t ) ); // restore  P_2
  pcb[ nextProcess ].status = STATUS_EXECUTING;            // update   P_2 status
  executing = nextProcess;                                 // update   index => P_2
  return;
}

extern void     main_P4(); 
extern uint32_t tos_P4;
extern void     main_P3(); 
extern uint32_t tos_P3;

void hilevel_handler_rst(ctx_t* ctx ) {
  /* Configure the mechanism for interrupt handling by
   *
   * - configuring timer st. it raises a (periodic) interrupt for each 
   *   timer tick,
   * - configuring GIC st. the selected interrupts are forwarded to the 
   *   processor via the IRQ interrupt signal, then
   * - enabling IRQ interrupts.
   */

  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  int_enable_irq();

  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 2;
  pcb[ 0 ].status   = STATUS_READY;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_P4 );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_P4 );

  memset( &pcb[ 1 ], 0, sizeof( pcb_t ) );
  pcb[ 1 ].pid      = 3;
  pcb[ 1 ].status   = STATUS_READY;
  pcb[ 1 ].ctx.cpsr = 0x50;
  pcb[ 1 ].ctx.pc   = ( uint32_t )( &main_P3 );
  pcb[ 1 ].ctx.sp   = ( uint32_t )( &tos_P3  );

  /* Once the PCBs are initialised, we (arbitrarily) select one to be
   * restored (i.e., executed) when the function then returns.
   */

  memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) );
  pcb[ 0 ].status = STATUS_EXECUTING;
  executing = 0;

  return;
}

void hilevel_handler_irq(ctx_t* ctx) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.

  if( id == GIC_SOURCE_TIMER0 ) {
    PL011_putc( UART0, 'T', true ); 
    scheduler( ctx );
    TIMER0->Timer1IntClr = 0x01;
  }

  // Step 5: write the interrupt identifier to signal we're done.

  GICC0->EOIR = id;

  return;
}

void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) { 
  /* Based on the identified encoded as an immediate operand in the
   * instruction, 
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */

  switch( id ) {
    case 0x00 : { // 0x00 => yield()
      scheduler( ctx );
      break;
    }

    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );  
      char*  x = ( char* )( ctx->gpr[ 1 ] );  
      int    n = ( int   )( ctx->gpr[ 2 ] ); 

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++, true );
      }
      
      ctx->gpr[ 0 ] = n;
      break;
    }

    case 0x02: { // read()
      int fd = (int)(ctx->gpr[0]);
      char* x = (char*)(ctx->gpr[1]);
      int n = (int)(ctx->gpr[2]);

      for(int i=0; i < n; i++){
        *x = PL011_getc(UART0,true);
        *x++;
      }
      ctx->gpr[0] = n;
      break;
    }

    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
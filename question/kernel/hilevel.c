#include "hilevel.h"
#define priority(process) (pcb[process].priority + (time - pcb[process].readyTime))

const int maxProcess = 100;
int numInQueue = 1;
pcb_t pcb[100];
int executing = 0;
int time = 0;
extern void     main_console(); 
extern uint32_t tos_main;

void scheduler( ctx_t* ctx ) {
  //Decide next executing process based on base priority and time they've been waiting
  int nextProcess = 0;
  for(int i = 0; i <numInQueue; i++){
    if(priority(nextProcess) < priority(i)){
        nextProcess = i;
    }
  }
  //Perform Context Switch
  memcpy( &pcb[ executing ].ctx, ctx, sizeof( ctx_t ) ); // preserve P_1
  pcb[ executing ].status = STATUS_READY;                // update   P_1 status
  pcb[ executing ].readyTime = time;
  memcpy( ctx, &pcb[ nextProcess ].ctx, sizeof( ctx_t ) ); // restore  P_2
  pcb[ nextProcess ].status = STATUS_EXECUTING;            // update   P_2 status
  
  executing = nextProcess;                                 // update   index => P_2
  time = (time+1)%maxProcess;
  return;
}

void addProcess(ctx_t* ctx, int priority){
  if(numInQueue == maxProcess){ 
    PL011_putc( UART0, 'X', true );
    return;
  }
  uint32_t newSP = &tos_main - numInQueue*(0x1000);
  //Copy the stack from the parent into new process
  uint32_t tos_current = &tos_main - executing*(0x1000);
  memcpy((void*)newSP,(void*)tos_current,tos_current-ctx->sp);
  //Create a PCB for the new process
  memset( &pcb[ numInQueue ], 0, sizeof( pcb_t ) );
  pcb[ numInQueue ].pid      = numInQueue;
  pcb[ numInQueue ].status   = STATUS_READY;
  pcb[ numInQueue ].ctx.pc   = ctx->pc;
  pcb[ numInQueue ].ctx.sp   = newSP;
  pcb[ numInQueue ].ctx.cpsr = ctx->cpsr;
  pcb[ numInQueue].ctx.lr    = ctx->lr;
  pcb[ numInQueue ].priority = priority;
  pcb[ numInQueue ].readyTime= time;
  for(int i = 1; i < 13; i++){
    pcb[numInQueue].ctx.gpr[i] = ctx->gpr[i];
  }

  numInQueue++;
  }

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
  pcb[ 0 ].pid      = 0;
  pcb[ 0 ].status   = STATUS_READY;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_main );
  pcb[ 0 ].priority = 1;
  pcb[ 0 ].readyTime = 0;


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
    case 0x03: { // fork()
      ctx_t* new_ctx  = ctx;
      new_ctx->gpr[0] = 0;
      ctx->gpr[0]     = 1;
      addProcess(new_ctx,1);
      break;
    }
    case 0x04: {//exit()
      int x = ctx->gpr[0];
      PL011_putc( UART0, x, true );
      //Set the PCB of the current process to 0
      memset(&pcb[executing],0,sizeof(pcb_t));
      //move the higher PCB down in the queue
      for(int i = executing+1 ; i< numInQueue;i++){
        pcb[i-1] = pcb[i];
      }
      numInQueue--;
      //Invoke the scheduler to select a new process to run
      scheduler(ctx);
      break;
    }
    case 0x05: {//exec
      ctx->sp = tos_main + executing*0x1000;
      const void* pc = (void*)(ctx->gpr[0]);
      for(int i = 0; i < 13; i++){
        ctx->gpr[i] = 0;
      }
      ctx->cpsr = 0x50;
      ctx->lr = (uint32_t)(pc); 
      //stop new processes getting back into their parent's memory
      break;
    }
    case 0x06: { //kill
      pid_t pid = (pid_t)(ctx->gpr[0]);
      int x = (int)(ctx->gpr[1]);
      pcb_t target = pcb[pid];
      PL011_putc( UART0, x, true );
      //Set the PCB of the current process to 0
      memset(&pcb[pid],0,sizeof(pcb_t));
      //move the higher PCB down in the queue
      for(int i = pid+1 ; i< numInQueue;i++){
        pcb[i-1] = pcb[i];
      }
      numInQueue--;
      //Invoke the scheduler to select a new process to run
      scheduler(ctx);
      break;
    }
    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static void consputc(int);

static int panicked = 0;

static struct
{
  struct spinlock lock;
  int locking;
} cons;

static void printint(int xx, int base, int sign)
{
  static char digits[] = "0123456789abcdef";
  char buf[16];
  int i;
  uint x;

  if (sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do
  {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);

  if (sign)
    buf[i++] = '-';

  while (--i >= 0)
    consputc(buf[i]);
}
// PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void cprintf(char *fmt, ...)
{
  int i, c, locking;
  uint *argp;
  char *s;

  locking = cons.locking;
  if (locking)
    acquire(&cons.lock);

  if (fmt == 0)
    panic("null fmt");

  argp = (uint *)(void *)(&fmt + 1);
  for (i = 0; (c = fmt[i] & 0xff) != 0; i++)
  {
    if (c != '%')
    {
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if (c == 0)
      break;
    switch (c)
    {
    case 'd':
      printint(*argp++, 10, 1);
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 's':
      if ((s = (char *)*argp++) == 0)
        s = "(null)";
      for (; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  if (locking)
    release(&cons.lock);
}

void panic(char *s)
{
  int i;
  uint pcs[10];

  cli();
  cons.locking = 0;
  // use lapiccpunum so that we can call panic from mycpu()
  cprintf("lapicid %d: panic: ", lapicid());
  cprintf(s);
  cprintf("\n");
  getcallerpcs(&s, pcs);
  for (i = 0; i < 10; i++)
    cprintf(" %p", pcs[i]);
  panicked = 1; // freeze other CPU
  for (;;)
    ;
}

// PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4
static ushort *crt = (ushort *)P2V(0xb8000); // CGA memory

static void cgaputc(int c)
{
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT + 1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT + 1);

  if (c == '\n')
    pos += 80 - pos % 80;
  else if (c == BACKSPACE)
  {
    if (pos > 0)
      --pos;
  }
  else
    crt[pos++] = (c & 0xff) | 0x0700; // black on white

  if (pos < 0 || pos > 25 * 80)
    panic("pos under/overflow");

  if ((pos / 80) >= 24)
  { // Scroll up.
    memmove(crt, crt + 80, sizeof(crt[0]) * 23 * 80);
    pos -= 80;
    memset(crt + pos, 0, sizeof(crt[0]) * (24 * 80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT + 1, pos >> 8);
  outb(CRTPORT, 15);
  outb(CRTPORT + 1, pos);
  crt[pos] = ' ' | 0x0700;
}

void consputc(int c)
{
  if (panicked)
  {
    cli();
    for (;;)
      ;
  }

  if (c == BACKSPACE)
  {
    uartputc('\b');
    uartputc(' ');
    uartputc('\b');
  }
  else
    uartputc(c);
  cgaputc(c);
}

#define INPUT_BUF 128
struct
{
  char buf[INPUT_BUF];
  uint r; // Read index
  uint w; // Write index
  uint e; // Edit index
} input;

#define C(x) ((x) - '@') // Control-x
#define TAB 0x09

// --- History support for Up/Down arrows ---
#define HIST_MAX 16
static char history[HIST_MAX][INPUT_BUF];
static int hist_count = 0; // number of entries currently stored (<= HIST_MAX)
static int hist_head = 0;  // next slot to write (ring buffer)
static int hist_pos =
    -1; // current browsing logical index ( -1 = not browsing )

static void history_add(void)
{
  // find start of the line we just finished (the last newline to current e)
  int start = input.e - 1;
  while (start > 0 && start > (int)input.r &&
         input.buf[(start - 1) % INPUT_BUF] != '\n')
    start--;
  int len = input.e - start;
  if (len <= 1) // empty line or only newline
    return;

  int copylen = len - 1; // exclude trailing '\n'
  if (copylen >= INPUT_BUF)
    copylen = INPUT_BUF - 1;

  int dst = hist_head;
  for (int i = 0; i < copylen; i++)
    history[dst][i] = input.buf[(start + i) % INPUT_BUF];
  history[dst][copylen] = '\0';

  hist_head = (hist_head + 1) % HIST_MAX;
  if (hist_count < HIST_MAX)
    hist_count++;
  hist_pos = -1; // reset browsing on new add
}

static void history_get(int logical_index, char *out)
{
  // logical_index: 0..hist_count-1 (0 oldest, hist_count-1 newest)
  if (logical_index < 0 || logical_index >= hist_count)
    return;

  // Compute real index: base = oldest = hist_head - hist_count
  int base = (hist_head - hist_count + HIST_MAX) % HIST_MAX;
  int real = (base + logical_index) % HIST_MAX;

  // Copy without using strcpy/string.h
  char *src = history[real];
  int i = 0;
  while (i < INPUT_BUF - 1 && src[i])
  {
    out[i] = src[i];
    i++;
  }
  out[i] = '\0';
}

static void history_restore(int logical_index)
{
  char tmp[INPUT_BUF];
  if (logical_index < 0)
    tmp[0] = '\0';
  else
    history_get(logical_index, tmp);

  // erase current edit line
  while (input.e != input.w)
  {
    input.e--;
    consputc(BACKSPACE);
  }

  // write the history line into the edit buffer and show it
  for (int i = 0; tmp[i] && input.e - input.r < INPUT_BUF; i++)
  {
    input.buf[input.e++ % INPUT_BUF] = tmp[i];
    consputc(tmp[i]);
  }
}

void autocomplete(void)
{
  char *line = &input.buf[input.r % INPUT_BUF];
  int len = input.e - input.r;

  char *commands[] = {"ls",
                      "cat",
                      "echo",
                      "forktest",
                      "grep",
                      "kill",
                      "mkdir",
                      "rm",
                      "pwd",
                      "cd",
                      "stressfs",
                      "wolfietest",
                      "clear",
                      "history",
                      "pwd",
                      "ps",
                      "setprioritytest",
                      "getstatstest",
                      "cpuhog",
                      0};

  char *match = 0;
  int match_count = 0;

  for (int i = 0; commands[i]; i++)
  {
    if (strncmp(commands[i], line, len) == 0)
    {
      match = commands[i];
      match_count++;
    }
  }

  if (match_count == 1)
  {
    for (int i = len; match[i]; i++)
    {
      input.buf[input.e++ % INPUT_BUF] = match[i];
      consputc(match[i]);
    }
  }
  else if (match_count > 1)
  {
    consputc('\n');
    for (int i = 0; commands[i]; i++)
    {
      if (strncmp(commands[i], line, len) == 0)
      {
        for (char *p = commands[i]; *p; p++)
          consputc(*p);
        consputc(' ');
      }
    }
    consputc('\n');
    consputc('$');
    consputc(' ');
    for (int i = input.r; i < input.e; i++)
      consputc(input.buf[i % INPUT_BUF]);
  }
}

void consoleintr(int (*getc)(void))
{
  int c, doprocdump = 0;

  acquire(&cons.lock);
  while ((c = getc()) >= 0)
  {
    // Handle ANSI arrow sequences: ESC [ A (up), ESC [ B (down)
    if (c == 0x1b) // ESC
    {
      int c1 = getc();
      if (c1 == '[')
      {
        int c2 = getc();
        if (c2 == 'A') // up
        {
          if (hist_count > 0)
          {
            if (hist_pos == -1)
              hist_pos = hist_count - 1;
            else if (hist_pos > 0)
              hist_pos--;
            history_restore(hist_pos);
          }
          continue;
        }
        else if (c2 == 'B') // down
        {
          if (hist_pos != -1)
          {
            if (hist_pos < hist_count - 1)
            {
              hist_pos++;
              history_restore(hist_pos);
            }
            else
            {
              // past newest -> clear to empty edit line
              hist_pos = -1;
              history_restore(-1);
            }
          }
          continue;
        }
        // unknown CSI sequence - ignore
      }
      // not a full escape sequence: fall through and ignore ESC
      continue;
    }

    switch (c)
    {
    case C('P'): // Process listing.
      // procdump() locks cons.lock indirectly; invoke later
      doprocdump = 1;
      break;
    case C('U'): // Kill line.
      while (input.e != input.w && input.buf[(input.e - 1) % INPUT_BUF] != '\n')
      {
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case C('H'):
    case '\x7f': // Backspace
      if (input.e != input.w)
      {
        input.e--;
        consputc(BACKSPACE);
      }
      break;
    case C('L'): // Ctrl + L -> Clear screen and redraw prompt
      consputc('\x1b');
      consputc('[');
      consputc('2');
      consputc('J');
      consputc('\x1b');
      consputc('[');
      consputc('H');

      consputc('$');
      consputc(' ');

      for (uint i = input.r; i < input.e; i++)
      {
        consputc(input.buf[i % INPUT_BUF]);
      }
      break;
    case TAB:
      autocomplete();
      break;

    default:
      if (c != 0 && input.e - input.r < INPUT_BUF)
      {
        c = (c == '\r') ? '\n' : c;

        // Stop history browsing when the user types a normal character
        hist_pos = -1;

        input.buf[input.e++ % INPUT_BUF] = c;
        consputc(c);
        if (c == '\n' || c == C('D') || input.e == input.r + INPUT_BUF)
        {
          // add to history when a line is submitted
          if (c == '\n')
            history_add();

          input.w = input.e;
          wakeup(&input.r);
        }
      }
      break;
    }
  }
  release(&cons.lock);
  if (doprocdump)
  {
    procdump(); // now call procdump() wo. cons.lock held
  }
}

int consoleread(struct inode *ip, char *dst, int n)
{
  uint target;
  int c;

  iunlock(ip);
  target = n;
  acquire(&cons.lock);
  while (n > 0)
  {
    while (input.r == input.w)
    {
      if (myproc()->killed)
      {
        release(&cons.lock);
        ilock(ip);
        return -1;
      }
      sleep(&input.r, &cons.lock);
    }
    c = input.buf[input.r++ % INPUT_BUF];
    if (c == C('D'))
    { // EOF
      if (n < target)
      {
        // Save ^D for next time, to make sure
        // caller gets a 0-byte result.
        input.r--;
      }
      break;
    }
    *dst++ = c;
    --n;
    if (c == '\n')
      break;
  }
  release(&cons.lock);
  ilock(ip);

  return target - n;
}

int consolewrite(struct inode *ip, char *buf, int n)
{
  int i;

  iunlock(ip);
  acquire(&cons.lock);
  for (i = 0; i < n; i++)
    consputc(buf[i] & 0xff);
  release(&cons.lock);
  ilock(ip);

  return n;
}

void consoleinit(void)
{
  initlock(&cons.lock, "console");

  devsw[CONSOLE].write = consolewrite;
  devsw[CONSOLE].read = consoleread;
  cons.locking = 1;

  ioapicenable(IRQ_KBD, 0);
}
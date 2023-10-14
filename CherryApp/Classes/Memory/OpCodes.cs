using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CherryApp.Classes.Memory
{
    public static class OpCodes
    {
        /* This OpCode Map Won't Work, Having The Same Values Will Still Draw A Key Already Exists Exception To Be Thrown. ~ Immune */
        public static readonly Dictionary<OpCode, byte> OpCodeMap = new Dictionary<OpCode, byte>()
        {
          {
            OpCode.Nop,
            (byte) 1
          },
          {
            OpCode.Ret,
            (byte) 1
          },
          {
            OpCode.Call,
            (byte) 5
          },
          {
            OpCode.Push,
            (byte) 5
          },
          {
            OpCode.Pop,
            (byte) 1
          },
          {
            OpCode.Add,
            (byte) 5
          },
          {
            OpCode.Sub,
            (byte) 5
          },
          {
            OpCode.Cmp,
            (byte) 5
          },
          {
            OpCode.JmpShort,
            (byte) 2
          },
          {
            OpCode.JnzShort,
            (byte) 2
          },
          {
            OpCode.JzShort,
            (byte) 2
          },
          {
            OpCode.JgShort,
            (byte) 2
          },
          {
            OpCode.JlShort,
            (byte) 2
          },
          {
            OpCode.JgeShort,
            (byte) 2
          },
          {
            OpCode.JleShort,
            (byte) 2
          },
          {
            OpCode.JmpNear,
            (byte) 5
          },
          {
            OpCode.JnzNear,
            (byte) 6
          },
          {
            OpCode.JzNear,
            (byte) 6
          },
          {
            OpCode.JgNear,
            (byte) 6
          },
          {
            OpCode.JlNear,
            (byte) 6
          },
          {
            OpCode.JgeNear,
            (byte) 6
          },
          {
            OpCode.JleNear,
            (byte) 6
          },
          {
            OpCode.JmpFar,
            (byte) 7
          },
          {
            OpCode.JnzNear,
            (byte) 7
          },
          {
            OpCode.JzNear,
            (byte) 7
          },
          {
            OpCode.JgNear,
            (byte) 7
          },
          {
            OpCode.JlNear,
            (byte) 7
          },
          {
            OpCode.JgeNear,
            (byte) 7
          },
          {
            OpCode.JleNear,
            (byte) 7
          },
          {
            OpCode.MovEax,
            (byte) 5
          },
          {
            OpCode.MovEcx,
            (byte) 5
          },
          {
            OpCode.MovEdx,
            (byte) 5
          },
          {
            OpCode.MovEbx,
            (byte) 5
          },
          {
            OpCode.MovEsp,
            (byte) 5
          },
          {
            OpCode.MovEbp,
            (byte) 5
          },
          {
            OpCode.MovEsi,
            (byte) 5
          },
          {
            OpCode.MovEdi,
            (byte) 5
          },
          {
            OpCode.PushEax,
            (byte) 1
          },
          {
            OpCode.PushEcx,
            (byte) 1
          },
          {
            OpCode.PushEdx,
            (byte) 1
          },
          {
            OpCode.PushEbx,
            (byte) 1
          },
          {
            OpCode.PushEsp,
            (byte) 1
          },
          {
            OpCode.PushEbp,
            (byte) 1
          },
          {
            OpCode.PushEsi,
            (byte) 1
          },
          {
            OpCode.PushEdi,
            (byte) 1
          },
          {
            OpCode.Pop,
            (byte) 1
          },
          {
            OpCode.PopEcx,
            (byte) 1
          },
          {
            OpCode.PopEdx,
            (byte) 1
          },
          {
            OpCode.PopEbx,
            (byte) 1
          },
          {
            OpCode.PopEsp,
            (byte) 1
          },
          {
            OpCode.PopEbp,
            (byte) 1
          },
          {
            OpCode.PopEsi,
            (byte) 1
          },
          {
            OpCode.PopEdi,
            (byte) 1
          },
          {
            OpCode.Pushad,
            (byte) 1
          },
          {
            OpCode.Popad,
            (byte) 1
          }
        };

        public enum OpCode
        {
            Add = 5,
            Sub = 45, // 0x0000002D
            Cmp = 61, // 0x0000003D
            PushEax = 80, // 0x00000050
            PushEcx = 81, // 0x00000051
            PushEdx = 82, // 0x00000052
            PushEbx = 83, // 0x00000053
            PushEsp = 84, // 0x00000054
            PushEbp = 85, // 0x00000055
            PushEsi = 86, // 0x00000056
            PushEdi = 87, // 0x00000057
            Pop = 88, // 0x00000058
            PopEax = 88, // 0x00000058
            PopEcx = 89, // 0x00000059
            PopEdx = 90, // 0x0000005A
            PopEbx = 91, // 0x0000005B
            PopEsp = 92, // 0x0000005C
            PopEbp = 93, // 0x0000005D
            PopEsi = 94, // 0x0000005E
            PopEdi = 95, // 0x0000005F
            Pushad = 96, // 0x00000060
            Popad = 97, // 0x00000061
            Push = 104, // 0x00000068
            JzShort = 116, // 0x00000074
            JnzShort = 117, // 0x00000075
            JlShort = 124, // 0x0000007C
            JgeShort = 125, // 0x0000007D
            JleShort = 126, // 0x0000007E
            JgShort = 127, // 0x0000007F
            Nop = 144, // 0x00000090
            MovEax = 184, // 0x000000B8
            MovEcx = 185, // 0x000000B9
            MovEdx = 186, // 0x000000BA
            MovEbx = 187, // 0x000000BB
            MovEsp = 188, // 0x000000BC
            MovEbp = 189, // 0x000000BD
            MovEsi = 190, // 0x000000BE
            MovEdi = 191, // 0x000000BF
            Ret = 195, // 0x000000C3
            Call = 232, // 0x000000E8
            JmpNear = 233, // 0x000000E9
            JmpFar = 234, // 0x000000EA
            JmpShort = 235, // 0x000000EB
            JzFar = 3972, // 0x00000F84
            JzNear = 3972, // 0x00000F84
            JnzFar = 3973, // 0x00000F85
            JnzNear = 3973, // 0x00000F85
            JlFar = 3980, // 0x00000F8C
            JlNear = 3980, // 0x00000F8C
            JgeFar = 3981, // 0x00000F8D
            JgeNear = 3981, // 0x00000F8D
            JleFar = 3982, // 0x00000F8E
            JleNear = 3982, // 0x00000F8E
            JgFar = 3983, // 0x00000F8F
            JgNear = 3983, // 0x00000F8F
        }
    }
}

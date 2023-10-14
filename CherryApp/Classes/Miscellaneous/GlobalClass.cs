using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CherryApp.Classes
{
    public static class GlobalClass
    {
        /* 
        This Is Entirely Pointless Cause Of It Being Async But If You 
        Really Wanted Global Do global::
        */

        public static CherryAPI CAPI = new CherryAPI();
    }
}

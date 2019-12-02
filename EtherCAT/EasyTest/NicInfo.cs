using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EasyTest
{
    // ネットワークインターフェース情報
    class NicInfo
    {
        public string Name;
        public string ID;

        public NicInfo(string name, string id)
        {
            Name = name;
            ID = id;
        }

        public override string ToString()
        {
            return Name;
        }
    }
}

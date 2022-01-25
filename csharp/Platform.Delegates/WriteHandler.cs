using System.Collections.Generic;

namespace Platform.Delegates
{
    public delegate TLinkAddress WriteHandler<TLinkAddress>?(IList<TLinkAddress>? before, IList<TLinkAddress>? after);
}

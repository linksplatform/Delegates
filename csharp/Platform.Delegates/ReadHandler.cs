using System.Collections.Generic;

namespace Platform.Delegates
{
    public delegate TLinkAddress ReadHandler<TLinkAddress>(IList<TLinkAddress>? link);
}

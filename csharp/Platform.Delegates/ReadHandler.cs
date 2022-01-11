using System.Collections.Generic;

namespace Platform.Delegates
{
    public delegate TLink ReadHandler<TLink>(IList<TLink> link);
}

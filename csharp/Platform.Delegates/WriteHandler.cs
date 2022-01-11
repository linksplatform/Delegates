using System.Collections.Generic;

namespace Platform.Delegates
{
    public delegate TLink WriteHandler<TLink>(IList<TLink> before, IList<TLink> after);
}

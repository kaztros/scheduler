#include <functional>

template <typename T>
void source_default (T trash) { /* Do nothing.  Or maybe debug? */ }

template <typename T>
struct SinkProtocol {
	virtual void sink (T) = 0;
};

template <typename T>
struct SourceDelegate {
	std::function <void (T)> source;

	SourceDelegate () : source (&source_default <T>) { }

	template <typename U>
	SourceDelegate (U x) : source (std::forward <U> (x)) { }
};

template <typename T>
struct ChannelProtocol
: SinkProtocol <T>
, SourceDelegate <T>
{ };

template <typename T>
struct ChannelIdentity
: ChannelProtocol <T>
{
	void sink (T x) override { this->source(x); }	
};


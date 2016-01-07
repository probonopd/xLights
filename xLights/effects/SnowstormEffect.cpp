#include "SnowstormEffect.h"
#include "SnowstormPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

SnowstormEffect::SnowstormEffect(int id) : RenderableEffect(id, "Snowstorm")
{
    //ctor
}

SnowstormEffect::~SnowstormEffect()
{
    //dtor
}

wxPanel *SnowstormEffect::CreatePanel(wxWindow *parent) {
    return new SnowstormPanel(parent);
}


class SnowstormClass
{
public:
    wxPointVector points;
    wxImage::HSVValue hsv;
    int idx,ssDecay;
    ~SnowstormClass()
    {
        points.clear();
    }
};

typedef std::list<SnowstormClass> SnowstormList;


// 0 <= idx <= 7
static wxPoint SnowstormVector(int idx)
{
    wxPoint xy;
    switch (idx)
    {
        case 0:
            xy.x=-1;
            xy.y=0;
            break;
        case 1:
            xy.x=-1;
            xy.y=-1;
            break;
        case 2:
            xy.x=0;
            xy.y=-1;
            break;
        case 3:
            xy.x=1;
            xy.y=-1;
            break;
        case 4:
            xy.x=1;
            xy.y=0;
            break;
        case 5:
            xy.x=1;
            xy.y=1;
            break;
        case 6:
            xy.x=0;
            xy.y=1;
            break;
        default:
            xy.x=-1;
            xy.y=1;
            break;
    }
    return xy;
}

static void SnowstormAdvance(RenderBuffer &buffer, SnowstormClass& ssItem)
{
    const int cnt = 8;  // # of integers in each set in arr[]
    const int arr[] = {30,20,10,5,0,5,10,20,20,15,10,10,10,10,10,15}; // 2 sets of 8 numbers, each of which add up to 100
    wxPoint adv = SnowstormVector(7);
    int i0 = ssItem.idx % 7 <= 4 ? 0 : cnt;
    int r=rand() % 100;
    for(int i=0, val=0; i < cnt; i++)
    {
        val+=arr[i0+i];
        if (r < val)
        {
            adv=SnowstormVector(i);
            break;
        }
    }
    if (ssItem.idx % 3 == 0)
    {
        adv.x *= 2;
        adv.y *= 2;
    }
    wxPoint xy=ssItem.points.back()+adv;
    xy.x %= buffer.BufferWi;
    xy.y %= buffer.BufferHt;
    if (xy.x < 0) xy.x+=buffer.BufferWi;
    if (xy.y < 0) xy.y+=buffer.BufferHt;
    ssItem.points.push_back(xy);
}


class SnowstormRenderCache : public EffectRenderCache {
public:
    SnowstormRenderCache() {};
    virtual ~SnowstormRenderCache() {};
    
    int LastSnowstormCount;
    SnowstormList SnowstormItems;
};


void SnowstormEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int Count = SettingsMap.GetInt("SLIDER_Snowstorm_Count", 0);
    int TailLength = SettingsMap.GetInt("SLIDER_Snowstorm_Length", 0);
    int sSpeed = SettingsMap.GetInt("SLIDER_Snowstorm_Speed", 0);

    // create new meteors
    wxImage::HSVValue hsv,hsv0,hsv1;
    buffer.palette.GetHSV(0,hsv0);
    buffer.palette.GetHSV(1,hsv1);
    SnowstormClass ssItem;
    wxPoint xy;
    int r;
    if (TailLength == 0) {
        TailLength = 1;
    }
    
    SnowstormRenderCache *cache = (SnowstormRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new SnowstormRenderCache();
        buffer.infoCache[id] = cache;
    }
    SnowstormList &SnowstormItems = cache->SnowstormItems;
    
    
    if (buffer.needToInit || Count != cache->LastSnowstormCount)
    {
        buffer.needToInit = false;
        // create snowstorm elements
        cache->LastSnowstormCount=Count;
        SnowstormItems.clear();
        for(int i=0; i<Count; i++)
        {
            ssItem.idx=i;
            ssItem.ssDecay=0;
            ssItem.points.clear();
            buffer.SetRangeColor(hsv0,hsv1,ssItem.hsv);
            // start in a random state
            r=rand() % (2*TailLength);
            if (r > 0)
            {
                xy.x=rand() % buffer.BufferWi;
                xy.y=rand() % buffer.BufferHt;
                ssItem.points.push_back(xy);
            }
            if (r >= TailLength)
            {
                ssItem.ssDecay = r - TailLength;
                r = TailLength;
            }
            for (int j=1; j < r; j++)
            {
                SnowstormAdvance(buffer, ssItem);
            }
            SnowstormItems.push_back(ssItem);
        }
    }
    
    // render Snowstorm Items
    int sz;
    int cnt=0;
    for (SnowstormList::iterator it=SnowstormItems.begin(); it!=SnowstormItems.end(); ++it)
    {
        if (it->points.size() > TailLength)
        {
            if (it->ssDecay > TailLength)
            {
                it->points.clear();  // start over
                it->ssDecay=0;
            }
            else if (rand() % 20 < sSpeed)
            {
                it->ssDecay++;
            }
        }
        if (it->points.empty())
        {
            xy.x=rand() % buffer.BufferWi;
            xy.y=rand() % buffer.BufferHt;
            it->points.push_back(xy);
        }
        else if (rand() % 20 < sSpeed)
        {
            SnowstormAdvance(buffer, *it);
        }
        sz=it->points.size();
        for(int pt=0; pt < sz; pt++)
        {
            hsv=it->hsv;
            if (buffer.allowAlpha) {
                xlColor c(hsv);
                c.alpha = 255.8 * (1.0 - double(sz - pt + it->ssDecay)/TailLength);
                buffer.SetPixel(it->points[pt].x,it->points[pt].y,c);
            } else {
                hsv.value=1.0 - double(sz - pt + it->ssDecay)/TailLength;
                if (hsv.value < 0.0) hsv.value=0.0;
                buffer.SetPixel(it->points[pt].x,it->points[pt].y,hsv);
            }
        }
        cnt++;
    }
}
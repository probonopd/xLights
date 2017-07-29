#ifndef CHANNELBLOCKMODEL_H
#define CHANNELBLOCKMODEL_H

#include "Model.h"
#include <vector>

class ChannelBlockModel : public ModelWithScreenLocation<TwoPointScreenLocation>
{
    public:
        ChannelBlockModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~ChannelBlockModel();
    
        virtual void GetBufferSize(const std::string &type, const std::string &transform,
                                   int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void DisableUnusedProperties(wxPropertyGridInterface *grid) override;

    protected:
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual int CalcCannelsPerString() override;

    private:
		void InitChannelBlock();
        static std::vector<std::string> LINE_BUFFER_STYLES;
        static std::string ChanColorAttrName(int idx)
        {
            return wxString::Format(wxT("ChannelColor%i"), idx + 1).ToStdString();  // a space between "String" and "%i" breaks the start channels listed in Indiv Start Chans
        }
        void AdjustChannelProperties(wxPropertyGridInterface *grid, int newNum);
};

#endif // CHANNELBLOCKMODEL_H
#include "PlayListItemAllOff.h"
#include <wx/xml/xml.h>
#include <wx/notebook.h>
#include "PlayListItemAllOffPanel.h"

PlayListItemAllOff::PlayListItemAllOff(wxXmlNode* node) : PlayListItem(node)
{
    _channels = 0;
    _startChannel = 1;
    _duration = 50;
    _value = 0;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    PlayListItemAllOff::Load(node);
}

void PlayListItemAllOff::Load(wxXmlNode* node) 
{
    PlayListItem::Load(node);
    _duration = wxAtoi(node->GetAttribute("Duration", "50"));
    _value = wxAtoi(node->GetAttribute("Value", "0"));
    _applyMethod = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", ""));
    _startChannel = wxAtoi(node->GetAttribute("StartChannel", "1"));
    _channels = wxAtoi(node->GetAttribute("Channels", "0"));
}

PlayListItemAllOff::PlayListItemAllOff() : PlayListItem()
{
    _channels = 0;
    _startChannel = 1;
    _duration = 50;
    _value = 0;
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    SetName("All Set");
}

PlayListItem* PlayListItemAllOff::Copy() const
{
    PlayListItemAllOff* res = new PlayListItemAllOff();
    res->_duration = _duration;
    res->_applyMethod = _applyMethod;
    res->_value = _value;
    res->_channels = _channels;
    res->_startChannel = _startChannel;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemAllOff::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIAllSet");

    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_duration));
    node->AddAttribute("Value", wxString::Format(wxT("%i"), _value));
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_applyMethod));
    node->AddAttribute("StartChannel", wxString::Format(wxT("%i"), (long)_startChannel));
    node->AddAttribute("Channels", wxString::Format(wxT("%i"), (long)_channels));

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemAllOff::GetTitle() const
{
    return "All Set";
}

void PlayListItemAllOff::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemAllOffPanel(notebook, this), GetTitle(), true);
}

void PlayListItemAllOff::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (outputframe)
    {
        if (ms >= _delay && ms <= _delay + _duration)
        {
            if (_startChannel > size) return;

            size_t toset = _channels + _startChannel - 1 < size ? _channels : size - _startChannel + 1;
            if (_channels == 0)
            {
                toset = size;
            }

            if (toset > 0)
            {
                wxByte* values = (wxByte*)malloc(toset);
                if (values != nullptr)
                {
                    memset(values, _value, toset);
                    Blend(buffer, size, values, toset, _applyMethod, _startChannel - 1);
                    free(values);
                }
            }
        }
    }
}


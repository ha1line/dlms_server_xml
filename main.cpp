#include <boost/foreach.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <iostream>
#include <string>

namespace bpt = boost::property_tree;

const char fileName[] = "/home/putin/Downloads/uspd1.ini";

namespace DlmsCosemXmlNames
{
	const std::string LINE = "lines.line";
	const std::string TYPE = "<xmlattr>.type";
	const std::string PORT = "<xmlattr>.port";
	const std::string BAUDRATE = "<xmlattr>.baudrate";
	const std::string LINE_ADDRESS = "<xmlattr>.address";
	const std::string LOGICAL_DEVICE = "logical_devices.logical_device";
	const std::string LOGICAL_ADDRESS = "<xmlattr>.address";
	const std::string CLASS_ID = "<xmlattr>.class_id";
	const std::string LOGICAL_NAME = "<xmlattr>.logical_name";
	const std::string DATA_SOURCE = "<xmlattr>.data_source";
	const std::string OBJECT = "object";
	const std::string SERVER = "servers.server";
	const std::string SERVERS = "servers";
	const std::string USPD_MODULE = "Module";
	const std::string USPD_TYPE = "TYPE";
} // namespace DlmsCosemXmlNames

struct DlmsCosemXmlHeader
{
	DlmsCosemXmlHeader();
	DlmsCosemXmlHeader(const std::string& type,
	                   const std::string& port,
	                   const std::string& baudrate,
	                   const std::string& lineAddress);

	const std::string m_type;
	const std::string m_port;
	const std::string m_baudrate;
	const std::string m_lineAddress;
};

struct ChannelObject
{
	/*
	Все, что идет в xml
	*/
	ChannelObject(const std::string dName,
	              const std::string id,
	              const std::string lName,
	              const std::string data,
	              int addend);
	std::string m_dlmsName;
	std::string m_classId;
	std::string m_logicalName;
	std::string m_dataSource; // type of channel
	int m_addend;
};

class ModuleObject
{
public:
	ModuleObject();
	ModuleObject(const std::string& type);

	void InsertChannel(const ChannelObject& channel);
	const std::vector<ChannelObject>& GetChannels() const;

private:
	const std::string m_type;
	std::vector<ChannelObject> m_channels;
};

class INIParser
{
public:
	explicit INIParser(const char* filename);
	const bpt::iptree& GetIniTree() const;
	const ModuleObject& GetModule(int moduleNumber) const;

private:
	std::vector<ModuleObject> m_modules;
	bpt::iptree m_iniTree;
};

class XMLParser
{
public:
	XMLParser(const INIParser& ini, const DlmsCosemXmlHeader& header);
	const bpt::iptree MakePTree() const;

private:
	void MakeXMLHeader() const;
	void MakeXMLLogicalDevice() const;
	void AddLogicalDevices() const;
	const INIParser& m_iniParser;
	const DlmsCosemXmlHeader m_header;
};

DlmsCosemXmlHeader::DlmsCosemXmlHeader()
	: m_type(""), m_port(""), m_baudrate(""), m_lineAddress("")
{
}

DlmsCosemXmlHeader::DlmsCosemXmlHeader(const std::string& type,
                                       const std::string& port,
                                       const std::string& baudrate,
                                       const std::string& lineAddress)
	: m_type(type), m_port(port), m_baudrate(baudrate), m_lineAddress(lineAddress)
{
}

ChannelObject::ChannelObject(const std::string dName,
                             const std::string id,
                             const std::string lName,
                             const std::string data,
                             int addend)
	: m_dlmsName(dName), m_classId(id), m_logicalName(lName), m_dataSource(data)
{
}

ModuleObject::ModuleObject()
{
}

ModuleObject::ModuleObject(const std::string& type)
	: m_type(type)
{
}

void ModuleObject::InsertChannel(const ChannelObject& channel)
{
	m_channels.push_back(channel);
}

const std::vector<ChannelObject>& ModuleObject::GetChannels() const
{
	return m_channels;
}

INIParser::INIParser(const char* filename)
{

	std::ifstream in(fileName);

	int m_dmCount = 0;

	try
	{
		if (in.is_open())
		{
			bpt::read_ini(in, m_iniTree);
			for (bpt::iptree::const_iterator it = m_iniTree.begin();
			     it != m_iniTree.end(); ++it)
			{
				if (it->first.find("Module") != std::string::npos)
				{
					bpt::iptree& module_tree(m_iniTree.get_child(it->first));
					std::cout << it->first << '\n';
					std::cout << module_tree.get<std::string>("TYPE") << '\n';
					m_modules.push_back({it->first});
				}

				if (it->first.find("B") != std::string::npos)
				{
					std::cout << it->first << '\n';

					bpt::iptree& module_tree(m_iniTree.get_child(it->first));
					std::cout << module_tree.get<std::string>("MODULE") << '\n';
				}
			}
			m_modules.shrink_to_fit();
		}
	}
	catch (bpt::ptree_error&)
	{
		std::cout << "Error" << '\n';
	}
	in.close();
}

const bpt::iptree& INIParser::GetIniTree() const
{
	return m_iniTree;
}

const ModuleObject& INIParser::GetModule(int moduleNumber) const
{
	//Нумерация модулей начинается с 1, элементы в векторе с 0
	return m_modules.at(--moduleNumber);
}

int main()
{
	INIParser iniParser = INIParser(fileName);

	return 0;
}

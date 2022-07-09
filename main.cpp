#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <fstream>
#include <iostream>
#include <string>

namespace bpt = boost::property_tree;

const char uspd_ini_path[] = "/home/putin/Downloads/uspd1.ini";
const char dlms_cosem_xml_path[] = "/home/putin/Downloads/xml_test.xml";

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
	DlmsCosemXmlHeader(const std::string& type, const std::string& port,
	                   const std::string& baudrate, const std::string& lineAddress);

	const std::string m_type;
	const std::string m_port;
	const std::string m_baudrate;
	const std::string m_lineAddress;
};

DlmsCosemXmlHeader dlmsCosemHeader = DlmsCosemXmlHeader("TCP/IP", "48999", "9600", "20");

struct ChannelObject
{
	ChannelObject();
	ChannelObject(std::string classId, const std::string& dlmsName, const std::string& type);

	std::string m_classId;
	std::string m_OBISCode;
	std::string m_type;
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
	explicit INIParser(const char* uspd_ini_path);
	const std::vector<ModuleObject>& GetModules() const;

private:
	ModuleObject& GetModule(int moduleNumber);
	std::vector<ModuleObject> m_modules;
};

class XMLParser
{
public:
	XMLParser(const INIParser& ini, const DlmsCosemXmlHeader& header);
	const bpt::iptree MakePTree() const;

private:
	void MakeXMLHeader(bpt::iptree& serverTree, const DlmsCosemXmlHeader& header) const;
	void MakeXMLLogicalDevices(bpt::iptree& serverTree, int lAdress) const;

	const INIParser& m_iniParser;
	const DlmsCosemXmlHeader& m_header;
};

DlmsCosemXmlHeader::DlmsCosemXmlHeader() : m_type(""), m_port(""), m_baudrate(""), m_lineAddress("")
{
}

DlmsCosemXmlHeader::DlmsCosemXmlHeader(const std::string& type, const std::string& port,
                                       const std::string& baudrate, const std::string& lineAddress)
	: m_type(type), m_port(port), m_baudrate(baudrate), m_lineAddress(lineAddress)
{
}

ChannelObject::ChannelObject() : m_classId(""), m_OBISCode(""), m_type("") {}

ChannelObject::ChannelObject(std::string classId, const std::string& dlmsName,
                             const std::string& type)
	: m_classId(classId), m_OBISCode(dlmsName), m_type(type)
{
}

ModuleObject::ModuleObject() {}

ModuleObject::ModuleObject(const std::string& type) : m_type(type) {}

void ModuleObject::InsertChannel(const ChannelObject& channel) { m_channels.push_back(channel); }

const std::vector<ChannelObject>& ModuleObject::GetChannels() const { return m_channels; }

INIParser::INIParser(const char* uspd_ini_path)
{
	bpt::iptree m_iniTree;

	std::ifstream in(uspd_ini_path);

	try
	{
		if (in.is_open())
		{
			bpt::read_ini(in, m_iniTree);

			for (bpt::iptree::const_iterator it = m_iniTree.begin(); it != m_iniTree.end(); ++it)
			{
				if (it->first.find("Module") != std::string::npos)
				{
					bpt::iptree& module_tree(m_iniTree.get_child(it->first));

					//std::cout << "MODULE: " << it->first << '\n';

					m_modules.push_back({it->first});

					const std::string moduleType = module_tree.get<std::string>("TYPE");

					/*
					if (moduleType.find("DM") != std::string::npos ||
					    moduleType.find("Sx") != std::string::npos)
					{
						std::cout << "TYPE: " << module_tree.get<std::string>("TYPE") << '\n';
					}
					*/
				}

				if (it->first.find("MODULE") == std::string::npos &&
				    it->first.find("USPD IN SYSTEM") == std::string::npos &&
				    it->first.find("ARCHIVES") == std::string::npos &&
				    it->first.find("COM") == std::string::npos)
				{
					bpt::iptree& channel_tree(m_iniTree.get_child(it->first));

					if (channel_tree.get("MODULE", 0))
					{

						ChannelObject cureentChannel;
						cureentChannel.m_type = it->first;

						if (channel_tree.get<std::string>("DlmsName", "") != "")
						{
							const std::string& dlmsName =
								channel_tree.get<std::string>("DlmsName", "");
							cureentChannel.m_classId = dlmsName.substr(0, dlmsName.find(','));
							cureentChannel.m_OBISCode =
								dlmsName.substr(dlmsName.find(',') + 1, dlmsName.size() - 1);
						}

						const int moduleNumber = channel_tree.get<int>("MODULE");
						ModuleObject& module = GetModule(moduleNumber);
						module.InsertChannel(cureentChannel);
						/*
						std::cout << "ClassId:" << cureentChannel.m_classId
								  << " OBISCode:" << cureentChannel.m_OBISCode
								  << " Type:" << cureentChannel.m_type << '\n';
						*/
					}
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

ModuleObject& INIParser::GetModule(int moduleNumber)
{
	//Нумерация модулей начинается с 1, элементы в векторе с 0
	return m_modules[--moduleNumber];
}

const std::vector<ModuleObject>& INIParser::GetModules() const { return m_modules; }

XMLParser::XMLParser(const INIParser& ini, const DlmsCosemXmlHeader& header)
	: m_iniParser(ini), m_header(header)
{
}

const bpt::iptree XMLParser::MakePTree() const
{
	const std::vector<ModuleObject>& modules(m_iniParser.GetModules());
	bpt::iptree xmlTree;
	try
	{
		bpt::iptree& defaultServerTree(xmlTree.add_child(DlmsCosemXmlNames::SERVER, bpt::iptree()));

		MakeXMLHeader(defaultServerTree, DlmsCosemXmlHeader());

		bpt::iptree& servetTree(xmlTree.add_child(DlmsCosemXmlNames::SERVER, bpt::iptree()));
		MakeXMLHeader(servetTree, m_header);
		MakeXMLLogicalDevices(servetTree, 16);
	}
	catch (bpt::ptree_error&)
	{
	}
	catch (boost::bad_lexical_cast&)
	{
	}

	return xmlTree;
}

void XMLParser::MakeXMLHeader(bpt::iptree& serverTree, const DlmsCosemXmlHeader& header) const
{
	bpt::iptree& lineTree(serverTree.add_child(DlmsCosemXmlNames::LINE, bpt::iptree()));
	lineTree.add(DlmsCosemXmlNames::TYPE, header.m_type);
	lineTree.add(DlmsCosemXmlNames::PORT, header.m_port);
	// Добавляем baudrate, если физический порт
	if (header.m_type == "serial")
	{
		lineTree.add(DlmsCosemXmlNames::BAUDRATE, header.m_baudrate);
	}
	lineTree.add(DlmsCosemXmlNames::LINE_ADDRESS, header.m_lineAddress);
}

void XMLParser::MakeXMLLogicalDevices(bpt::iptree& serverTree, int lAdress) const
{
	const std::vector<ModuleObject>& modules(m_iniParser.GetModules());

	for (size_t i = 0; i < modules.size(); ++i)
	{
		const std::vector<ChannelObject>& channels(modules.at(i).GetChannels());

		if (channels.size())
		{
			bpt::iptree& logicalDeviceTree(
				serverTree.add_child(DlmsCosemXmlNames::LOGICAL_DEVICE, bpt::iptree()));
			logicalDeviceTree.add(DlmsCosemXmlNames::LOGICAL_ADDRESS, lAdress);
			++lAdress;

			for (size_t j = 0; j < channels.size(); ++j)
			{
				bpt::iptree& currentChannel =
					logicalDeviceTree.add_child(DlmsCosemXmlNames::OBJECT, bpt::iptree());
				currentChannel.add(DlmsCosemXmlNames::CLASS_ID, channels.at(j).m_classId);
				currentChannel.add(DlmsCosemXmlNames::LOGICAL_NAME, channels.at(j).m_OBISCode);
				currentChannel.add(DlmsCosemXmlNames::DATA_SOURCE, channels.at(j).m_type);
			}
		}
	}
}

int main()
{
	INIParser iniParser = INIParser(uspd_ini_path);
	XMLParser xmlParser = XMLParser(iniParser, dlmsCosemHeader);
	const bpt::iptree& xmlTree = xmlParser.MakePTree();

	std::ofstream out(dlms_cosem_xml_path, std::ios_base::trunc | std::ios_base::out);

	if (out)
	{
		bpt::write_xml(out, xmlTree, bpt::xml_writer_settings<std::string>(' ', 2));
	}
	out.close();

	return 0;
}

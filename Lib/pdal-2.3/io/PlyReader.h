#ifndef PLYREADER_H
#define PLYREADER_H

#include <string>
#include <stack>

#include <pdal/Dimension.h>
#include <pdal/Reader.h>
#include <pdal/StageFactory.h>
#include <pdal/Streamable.h>

namespace pdal
{
	class PlyReader : public Reader, public Streamable
	{
	public:
		std::string getName() const;

		typedef std::map<std::string, Dimension::Id> DimensionMap;

		PlyReader();

	private:
		enum class Format
		{
			Ascii,
			BinaryLe,
			BinaryBe
		};

		struct Property
		{
			Property(const std::string& name) : m_name(name)
			{}
			virtual ~Property()
			{}

			std::string m_name;

			virtual void setDim(Dimension::Id id)
			{}
			virtual void read(std::istream* stream, PlyReader::Format format,
				PointRef& point) = 0;
		};

		struct SimpleProperty : public Property
		{
			SimpleProperty(const std::string& name, Dimension::Type type) :
				Property(name), m_type(type), m_dim(Dimension::Id::Unknown)
			{}

			Dimension::Type m_type;
			Dimension::Id m_dim;

			virtual void read(std::istream* stream, PlyReader::Format format,
				PointRef& point) override;
			virtual void setDim(Dimension::Id id) override
			{
				m_dim = id;
			}
		};

		struct ListProperty : public Property
		{
			ListProperty(const std::string& name, Dimension::Type countType,
				Dimension::Type listType) : Property(name), m_countType(countType),
				m_listType(listType)
			{}

			Dimension::Type m_countType;
			Dimension::Type m_listType;

			virtual void read(std::istream* stream, PlyReader::Format format,
				PointRef& point) override;
		};

		struct Element
		{
			Element(const std::string name, size_t count) :
				m_name(name), m_count(count)
			{}

			std::string m_name;
			size_t m_count;
			std::vector<std::unique_ptr<Property>> m_properties;
		};

		Format m_format;
		std::string m_line;
		std::string::size_type m_linePos;
		std::stack<std::string> m_lines;
		std::istream* m_stream;
		std::istream::pos_type m_dataPos;
		std::vector<Element> m_elements;
		PointId m_index;
		Element* m_vertexElt;

		virtual void initialize();
		virtual void addDimensions(PointLayoutPtr layout);
		virtual void ready(PointTableRef table);
		virtual point_count_t read(PointViewPtr view, point_count_t num);
		virtual void done(PointTableRef table);
		virtual bool processOne(PointRef& point);

		std::string readLine();
		void pushLine();
		std::string nextWord();
		void extractMagic();
		void extractEnd();
		void extractFormat();
		Dimension::Type getType(const std::string& name);
		void extractProperty(Element& element);
		void extractProperties(Element& element);
		bool extractElement();
		void extractHeader();
		void readElement(Element& elt, PointRef& point);
		bool readProperty(Property* prop, PointRef& point);
	};
}

#endif // #ifndef PLYREADER_H

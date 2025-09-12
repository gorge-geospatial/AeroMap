#ifndef GLTFWRITER_H
#define GLTFWRITER_H

#include <pdal/Writer.h>

namespace pdal
{
	class OLeStream;

	typedef std::shared_ptr<std::ostream> FileStreamPtr;

	class GltfWriter : public Writer
	{
		struct ViewData;

	public:
		GltfWriter();
		~GltfWriter();
		GltfWriter(const GltfWriter&) = delete;
		GltfWriter& operator=(const GltfWriter&) = delete;

		std::string getName() const;

	private:
		virtual void addArgs(ProgramArgs& args);
		virtual void ready(PointTableRef table);
		virtual void write(const PointViewPtr v);
		virtual void done(PointTableRef table);
		virtual void prepared(PointTableRef table);

		void writeGltfHeader();
		void writeJsonChunk();
		void writeBinHeader();

		std::string m_filename;
		std::unique_ptr<OLeStream> m_stream;
		std::vector<ViewData> m_viewData;
		size_t m_totalSize;
		size_t m_binSize;
		bool m_writeNormals;

		double m_metallic;
		double m_roughness;
		double m_red;
		double m_green;
		double m_blue;
		double m_alpha;
		bool m_doubleSided;
		bool m_colorVertices;
	};

	struct GltfWriter::ViewData
	{
		BOX3D m_bounds;
		size_t m_indexOffset;
		size_t m_indexByteLength;
		size_t m_indexCount;
		size_t m_vertexOffset;
		size_t m_vertexByteLength;
		size_t m_vertexCount;
	};
}

#endif // #ifndef GLTFWRITER_H

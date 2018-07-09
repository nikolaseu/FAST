%include "FAST/Visualization/Renderer.i"
%shared_ptr(fast::SliceRenderer)

namespace fast {

enum PlaneType {PLANE_X, PLANE_Y, PLANE_Z};

class SliceRenderer : public Renderer {
    public:
    	static std::shared_ptr<SliceRenderer> New();
        void setInputConnection(ProcessObjectPort port);
        void setSliceToRender(unsigned int sliceNr);
        void setSlicePlane(PlaneType plane);
	private:
        SliceRenderer();
};

%template(SliceRendererPtr) std::shared_ptr<SliceRenderer>;

}
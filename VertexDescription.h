
#include "LinearAlgebra.h"

namespace VertexDescriptions {	
	struct POS2_COLOR3 {
		Vector2 position;
		Vector3 color;
	};

	struct TRANSFORM_MODEL_VIEW {
		Mat3 model;
		Mat3 view;
	};
}
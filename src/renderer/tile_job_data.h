#pragma once

#include "renderer/render_context.h"

namespace raytracer {

	struct TileJobData {
		const RenderContext* ctx;
		int x_start;
		int x_end;
		int y_start;
		int y_end;
	};

} // namespace raytracer
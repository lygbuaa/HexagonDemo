#include "Halide.h"

namespace {

class HalideRemap : public Halide::Generator<HalideRemap> {
private:
    Var x{"x"}, y{"y"}, c{"c"};

public:
    Input<Buffer<uint8_t>>  inputImage{"inputImage", 3};
//    Input<Buffer<uint16_t>>  tableX{"tableX", 2};
//    Input<Buffer<uint16_t>>  tableY{"tableY", 2};
    Output<Buffer<uint8_t>> outputImage{"outputImage", 3};

    void generate() {
        // The algorithm
//        Expr nnx = tableX(x, y);
//        Expr nny = tableY(x, y);
//        outputImage(x, y, c) = (inputImage(nnx, nny, c) + inputImage(nnx+1, nny, c) + inputImage(nnx, nny+1, c) + inputImage(nnx+1, nny+1, c)) << 2;
        outputImage(x, y, c) = inputImage(x, y, c);
    }

    void schedule() {
        Var xi{"xi"}, yi{"yi"};
        inputImage.dim(0).set_bounds_estimate(0, 640);
        inputImage.dim(1).set_bounds_estimate(0, 480);
        inputImage.dim(2).set_bounds_estimate(0, 3);

        auto output_buffer = Func(outputImage).output_buffer();
        output_buffer.dim(0).set_bounds_estimate(0, 640);
        output_buffer.dim(1).set_bounds_estimate(0, 480);
        output_buffer.dim(2).set_bounds_estimate(0, 3);

        // How to schedule it
        if (get_target().features_any_of({Target::HVX_128})) {
            const int vector_size = 128;
            Expr input_stride = inputImage.dim(1).stride();
            inputImage.dim(1).set_stride((input_stride/vector_size) * vector_size);
            inputImage.set_host_alignment(vector_size);

            Expr output_stride = output_buffer.dim(1).stride();
            output_buffer.dim(1).set_stride((output_stride/vector_size) * vector_size);
            output_buffer.set_host_alignment(vector_size);

            Func(outputImage)
                    .tile(x, y, xi, yi, vector_size, 4, TailStrategy::RoundUp)
                    .vectorize(xi)
                    .unroll(yi);
        } else {
            const int vector_size = natural_vector_size<uint8_t>();
            Func(outputImage)
                    .vectorize(x, vector_size)
                    .parallel(y, 16);
        }
//        std::vector<Halide::Argument> args = {input};
//        blur_x.compile_to_lowered_stmt("blur_x.html", args, Halide::HTML);
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(HalideRemap, halide_remap)

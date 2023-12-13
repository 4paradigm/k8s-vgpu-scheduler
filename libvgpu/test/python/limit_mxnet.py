import sys
import argparse
import mxnet as mx
from mxnet import nd


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--device", type=int, default=0,
                        help="Test device id")
    parser.add_argument("--tensor_shape", type=str, default=None,
                        help="Specify device tensor shape for test")
    args = parser.parse_args(sys.argv[1:])    
    return args


def app(args):
    if args.tensor_shape is None:
        args.tensor_shape = "1024, 1024, 1024"
    shape = [int(_.strip()) for _ in args.tensor_shape.split(",") if _.strip() != ""]
    print("Test use float device tensor: (%s)" % ", ".join([str(_) for _ in shape])) 

    gpu_device = mx.gpu(args.device)
    with mx.Context(gpu_device):
        cpu_tensor = mx.nd.ones(shape, mx.cpu())
        gpu_tensor = mx.nd.ones(shape, gpu_device)
        cpu_tensor.copyto(gpu_tensor)
        print("Tensor sum: " + str(gpu_tensor.sum().asnumpy()))


if __name__ == "__main__":
    app(parse_args())


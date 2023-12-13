import sys
import argparse
import torch


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

    print("PyTorch version: " + torch.__version__)
    data = torch.ones(size=shape, dtype=torch.float32).cuda()
    summation = data.sum()
    print("Tensor sum: " + str(summation.cpu().numpy()))


if __name__ == "__main__":
    app(parse_args())


import sys
import argparse
import numpy as np
import tensorflow as tf


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--device", type=int, default=0,
                        help="Test device id")
    parser.add_argument("--tensor_shape", type=str, default=None,
                        help="Specify device tensor shape for test")
    args = parser.parse_args(sys.argv[1:])    
    return args


def app(args):
    with tf.Session() as sess:
        if args.tensor_shape is not None:
            shape = [int(_.strip()) for _ in args.tensor_shape.split(",") if _.strip() != ""]
            tensor = tf.placeholder(dtype=tf.float32, shape=shape)
            print ("Input tensor shape: " + str(shape))
            
            with tf.device("/gpu:%d" % args.device):
                result = tf.reduce_sum(tensor)
            
            sess.run(result, feed_dict={
                tensor: np.random.rand(*shape)
            })


if __name__ == "__main__":
    app(parse_args())


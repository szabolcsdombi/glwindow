import glwindow
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('module', nargs='?', default='main')
args = parser.parse_args()

glwindow.run(args.module)

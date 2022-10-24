#!/usr/bin/python
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--before", help="Configuration before the transaction.", required=True)
parser.add_argument("--after", help="Configuration after the transaction.", required=True)

args = parser.parse_args()

print("Before:")
f=open(args.before,"r")
print((f.read()))

print("After:")
f=open(args.after,"r")
print((f.read()))


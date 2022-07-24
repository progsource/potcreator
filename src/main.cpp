#include <iostream>

#include <thread>
#include <chrono>

#include "potcreator/potcreator.h"

int main(int argc, char** argv)
{
  ps::potcreator::PotCreator potc;
  return potc.run(argc, argv);
}

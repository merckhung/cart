import java.util.Random;

class Mem {

  static Test[] abc;
  static Test[] ccc;
  static Test[] ddd;

  public static void main(String args[]) {
    Random rnd = new Random();

    while (true) {
      int a = rnd.nextInt(4096);
      int b = rnd.nextInt(8192);
      int c = rnd.nextInt(10240);

      abc = new Test[a];
      ccc = new Test[b];
      ddd = new Test[c];
    }
  }
};


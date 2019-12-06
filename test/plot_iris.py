import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

def main():
    df = pd.read_csv("../data/iris.csv")
    sns.relplot(
        data = df,
        x = "sepal_length",
        y = "petal_length",
        hue = "species",
    )
    plt.show()

if __name__ == "__main__":
    main()

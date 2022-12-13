from src.soup import Soup

def main():
    """Runs the Soup simulation.
    """
    soup = Soup(terms=10000, alphabet="BCKW")
    for i in range(1000):
        soup.step()
        if i % 1 == 0:
            print(f"STEP {i}.")
            print(f"  immortals: {soup.immortals()}")

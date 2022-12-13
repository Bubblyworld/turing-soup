from src.soup import Soup

def main():
    """Runs the Soup simulation.
    """
    soup = Soup(terms=100, alphabet="SKI")
    for i in range(100000):
        soup.step()
        if i % 100 == 0:
            print(f"STEP {i}: {soup}")

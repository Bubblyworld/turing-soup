from src.soup import Soup

def main():
    """Runs the Soup simulation.
    """
    soup = Soup(terms=2000, alphabet="SKI")
    for i in range(1000):
        soup.step()
        print(f"STEP {i}: {soup}")
